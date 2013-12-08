/* Copyright (C) 2011 Enrico Rossi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include "i2c.h"

uint8_t i2c_send(const uint8_t code, const uint8_t data)
{
	switch (code) {
		case START:
			TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
			loop_until_bit_is_set(TWCR, TWINT);
			break;
		case STOP:
			TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
			break;
		case SLA:
			TWDR = data;
			/* clear interrupt to start transmission */
			TWCR = _BV(TWINT) | _BV(TWEN); 
			loop_until_bit_is_set(TWCR, TWINT);
			break;
		case DATA:
			TWDR = data;
			/* clear interrupt to start transmission */
			TWCR = _BV(TWINT) | _BV(TWEN); 
			loop_until_bit_is_set(TWCR, TWINT);
			break;
		case ACK:
			TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
			loop_until_bit_is_set(TWCR, TWINT);
			break;
		case NACK:
			TWCR = _BV(TWINT) | _BV(TWEN);
			loop_until_bit_is_set(TWCR, TWINT);
			break;
		default:
			break;
	}

	return(TW_STATUS);
}

/** Initialize the i2c bus.
 *
 * See the datasheet for SCL speed.
 *
 * SCL freq = CPU FREQ / (16 + 2 * TWBR * Prescaler)
 *
 * @note 16Mhz Arduino, 100Khz I2C bus, Prescaler = 4, TWBR = 18
 */
void i2c_init(void)
{
	/* Prescaler 4 */
	TWSR = _BV(TWPS0);
	TWBR = 18;
}

/** Send a byte to the i2c slave.
 * @param addr address of the slave.
 * @param data byte to send.
 * @param stop if true send the stop condition at the end.
 * @note Atmel does not show the skip of stop in transitions.
 */
uint8_t i2c_master_send_b(const uint8_t addr, const uint8_t data,
		const uint8_t stop)
{
	uint8_t err;

	/* Send ADDR_W reg. 0 req. */
	err = i2c_send(START, 0);

	if ((err == TW_START) || (err == TW_REP_START))
		err = i2c_send(SLA, addr | WRITE);

	if (err == TW_MT_SLA_ACK)
		err = i2c_send(DATA, data);

	if (err == TW_MT_DATA_ACK)
		err = 0;

	if (stop)
		i2c_send(STOP, 0);

	return(err);
}

/*! \brief i2c master send a word.
 * Send a word to the i2c slave with stop at the end.
 * \param addr address of the slave.
 * \param msb byte to send first.
 * \param lsb byte to send last.
 * \return 0 - OK, 1 - Error
 */
uint8_t i2c_master_send_w(const uint8_t addr, const uint8_t msb, const uint8_t lsb)
{
	uint8_t err;

	/* Send ADDR_W reg. 0 req. */
	err = i2c_send(START, 0);

	if ((err == TW_START) || (err == TW_REP_START))
		err = i2c_send(SLA, addr | WRITE);

	if (err == TW_MT_SLA_ACK)
		err = i2c_send(DATA, msb);

	if (err == TW_MT_DATA_ACK)
		err = i2c_send(DATA, lsb);

	if (err == TW_MT_DATA_ACK)
		err = 0;

	i2c_send(STOP, 0);
	return(err);
}

/*! \brief i2c master read byte.
 * Read a byte from the slave with a stop at the end.
 * \param addr address of the slave.
 * \param byte pre-allocated byte.
 * \return 1 - value OK, 0 - Error.
 */
uint8_t i2c_master_read_b(const uint8_t addr, uint8_t *byte,
		const uint8_t stop)
{
	uint8_t err;

	err = i2c_send(START, 0);

	if ((err == TW_START) || (err == TW_REP_START))
		err = i2c_send(SLA, addr | READ);

	/* send an ACK to start the TX */
	if (err == TW_MR_SLA_ACK)
		err = i2c_send(ACK, 0);

	if (err == TW_MR_DATA_ACK) {
		*byte = TWDR;
		err = i2c_send(NACK, 0);
	}
	
	if (err == TW_MR_DATA_NACK)
		err = 0;

	if (stop)
		i2c_send(STOP, 0);

	return(err);
}
/*! Read a word (2 byte) from the slave.
 * \param addr address of the slave.
 * \param pre-allocated word.
 * \return 1 - value OK, 0 - Error.
 */
uint8_t i2c_master_read_w(const uint8_t addr, uint16_t *code)
{
	uint8_t err;

	*code = 0;
	err = i2c_send(START, 0);

	if ((err == TW_START) || (err == TW_REP_START))
		err = i2c_send(SLA, addr | READ);

	if (err == TW_MR_SLA_ACK) {
		/* send an ACK to start the TX */
		err = i2c_send(ACK, 0);
	}

	if (err == TW_MR_DATA_ACK) {
		/* read the first data msb */
		*code = (TWDR << 8);
		err = i2c_send(ACK, 0);
	}

	if (err == TW_MR_DATA_ACK) {
		/* read lsb */
		*code |= TWDR;
		err = i2c_send(NACK, 0);
	}

	if (err == TW_MR_DATA_NACK)
		err = 0;

	i2c_send(STOP, 0);
	return(err);
}
