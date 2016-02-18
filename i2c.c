/* Copyright (C) 2011-2016 Enrico Rossi
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

#include <stdint.h>
#include <util/twi.h>
#include <avr/io.h>
#include "i2c.h"

/*! Perform an i2c operation.
 *
 * \return the i2c status register properly masked.
 */
uint8_t i2c_send(const uint8_t code, const uint8_t data)
{
	switch (code) {
		/* valid also as restart */
		case START:
			TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
			loop_until_bit_is_set(TWCR, TWINT);
			break;
		case STOP:
			TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
			break;
		case SLA:
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

	return(TW_STATUS & 0xf8);
}

/*! Initialize the i2c bus.
 *
 * See the datasheet for SCL speed.
 * SCL freq = CPU FREQ / (16 + 2 * TWBR * Prescaler)
 * SCLf(max) = CPUf/16
 *
 * 16Mhz CLK, 100Khz I2C bus, Prescaler = 4, TWBR = 18
 * 1Mhz CLK, 10Khz I2C bus, Prescaler = 1, TWBR = 42
 */
void i2c_init(void)
{
	/* Prescaler 1 */
	TWSR = 0;
	TWBR = 42;
}

/*! Shutdown the i2c bus.
 */
void i2c_shut(void)
{
	TWSR = 0;
	TWBR = 0;
}

/*! i2c Master Trasmitter Mode.
 *
 * \param addr the i2c slave address.
 * \param lenght the number of byte to send.
 * \param *data the pointer to the block of byte.
 * \param stop the stop at the end of the communication.
 *
 */
uint8_t i2c_mtm(const uint8_t addr, const uint16_t lenght,
		uint8_t *data, uint8_t stop)
{
	uint16_t i;
	uint8_t err;

	/* START */
	err = i2c_send(START, 0);

	/* if start acknoledge */
	if ((err == TW_START) || (err == TW_REP_START))
		/* Send address - write */
		err = i2c_send(SLA, addr | WRITE);

	/* if the address is ACK */
	if (err == TW_MT_SLA_ACK)
		/* send data */
		for (i=0; i<lenght; i++) {
			err = i2c_send(DATA, *(data+i));

			/* if data is not ACK */
			if (err != TW_MT_DATA_ACK)
				/* exit */
				i=lenght;
		}

	/* if client NACK on ADDR or DATA */
	if ((err == TW_MT_SLA_NACK) || (err == TW_MT_DATA_NACK))
		/* send the stop */
		stop = TRUE;

	/* if data is ACK */
	if (err == TW_MT_DATA_ACK)
		err = 0;

	/* send the STOP if required */
	if (stop)
		i2c_send(STOP, 0);

	return(err);
}

/*! i2c Master Receiver Mode.
 *
 * \param addr the i2c slave address.
 * \param the max lenght the number of byte to receive.
 * \param *data the pointer to the block of byte.
 * \param stop the stop at the end of the communication.
 *
 */
uint8_t i2c_mrm(const uint8_t addr, const uint16_t lenght,
		uint8_t *data, uint8_t stop)
{
	uint16_t i;
	uint8_t err;

	/* START */
	err = i2c_send(START, 0);

	/* if start is sent */
	if ((err == TW_START) || (err == TW_REP_START))
		/* Send address - read */
		err = i2c_send(SLA, addr | READ);

	/* if the address is ACK */
	if (err == TW_MR_SLA_ACK)
		/* Receive data */
		for (i=0; i<lenght; i++) {
			/* send ACK */
			err = i2c_send(ACK, 0);

			/* if data is not ACK */
			if (err == TW_MR_DATA_ACK)
				*(data+i) = TWDR;
			else
				i = lenght;
		}

	/* Error NACK on ADDR-R or Last DATA */
	if ((err == TW_MR_SLA_NACK) || (err == TW_MR_DATA_NACK))
		/* send the stop */
		stop = TRUE;

	if (err == TW_MR_DATA_ACK) {
		/* last byte, send NACK */
		err = i2c_send(NACK, 0);

		/* if data is NACK */
		if (err == TW_MR_DATA_NACK)
			err = 0;
	}

	/* send the STOP if required */
	if (stop)
		i2c_send(STOP, 0);

	return(err);
}

/*! I2C General Call
 *
 * ex. i2c_gc(I2C_GC_RESET);
 */
uint8_t i2c_gc(const uint8_t call)
{
	uint8_t i, err;
	err = 0;

	switch(call) {
		case I2C_GC_RESET:
		default:
			/* Send the General Call reset */
			i = 0x06;
			err = i2c_mtm(0, 1, &i, TRUE);
	}

	return(err);
}

#ifdef I2C_LEGACY_MODE

/*! Legacy master Send a byte.
 *
 * \param addr address of the slave.
 * \param data byte to send.
 * \param stop if true send the stop condition at the end.
 * \return 0 = OK or error.
 * \note Atmel does not show the skip of stop in transitions.
 */
uint8_t i2c_master_send_b(const uint8_t addr, const uint8_t data,
		uint8_t stop)
{
	return(i2c_mtm(addr, 1, (uint8_t *) &data, stop));
}

/*! legacy i2c master send a word.
 * Send a word to the i2c slave with stop at the end.
 * \param addr address of the slave.
 * \param msb byte to send first.
 * \param lsb byte to send last.
 * \return 0 - OK, 1 - Error
 */
uint8_t i2c_master_send_w(const uint8_t addr, const uint8_t msb,
		const uint8_t lsb)
{
	uint8_t err;
	uint16_t data;

	data = (msb << 8) | lsb;
	err = i2c_mtm(addr, 2, (uint8_t *) &data, TRUE);
	return(err);
}

/*! Legacy Master Read a byte.
 *
 * \param addr address of the slave.
 * \param data pointer to the byte to receive.
 * \param stop if true send the stop condition at the end.
 * \return 0 = OK or error.
 * \note Atmel does not show the skip of stop in transitions.
 */
uint8_t i2c_master_read_b(const uint8_t addr, uint8_t *data,
		uint8_t stop)
{
	return(i2c_mrm(addr, 1, data, stop));
}

/*! Read a word (2 byte) from the slave.
 * \param addr address of the slave.
 * \param data pre-allocated word.
 * \return 1 - value OK, 0 - Error.
 */
uint8_t i2c_master_read_w(const uint8_t addr, uint16_t *data)
{
	uint8_t err;
	uint8_t swap;

	err = i2c_mrm(addr, 2, (uint8_t *) data, TRUE);
	/* swap lsb <-> msb */
	swap = *data & 0xff;
	*data = (*data << 8) | swap;

	return(err);
}

#endif
