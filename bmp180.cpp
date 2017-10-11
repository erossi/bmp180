/* Copyright (C) 2013, 2017 Enrico Rossi
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

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "bmp180.h"

/** Swap MSB<->LSB of an uint16.
 *
 * @param swapme ptr to the uint16.
 */
void swapit(uint16_t *swapme)
{
	uint8_t lsb;

	lsb = (*swapme) & 0xff;
	(*swapme) = (*swapme) >> 8;
	(*swapme) |= (uint16_t)lsb << 8;
}

/** Register Read (Byte).
 *
 * @param reg_addr the register address.
 * @param byte the data to be read.
 */
uint8_t BMP180::register_rb(uint8_t reg_addr, uint8_t *byte)
{
	uint8_t error;

	// do not STOP the tx
	error = i2c.tx(WRITE, 1, &reg_addr, false);

	if (!error)
		error = i2c.tx(READ, 1, byte);

	return (error);
}

/** Register Read (word).
 *
 * @param reg_addr the register address.
 * @param byte the data to be read.
 */
uint8_t BMP180::register_rw(uint8_t reg_addr, uint16_t *word)
{
	uint8_t err;

	// do not STOP the tx
	err = i2c.tx(WRITE, 1, &reg_addr, false);

	if (!err)
		err = i2c.tx(READ, 2, (uint8_t*) word);

	return (err);
}

/** Read the calibration data.
 *
 * \note: can this be a single data read from the
 * device's memory?
 */
uint8_t BMP180::dump_calibration_data(void)
{
	uint8_t err;
	uint16_t word;

	err = register_rw(BMP180_REG_AC1, &word);
	AC1 = (int16_t) word;

	if (!err) {
		err = register_rw(BMP180_REG_AC2, &word);
		AC2 = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_AC3, &word);
		AC3 = (int16_t) word;
	}

	if (!err)
		err = register_rw(BMP180_REG_AC4, &AC4);

	if (!err)
		err = register_rw(BMP180_REG_AC5, &AC5);

	if (!err)
		err = register_rw(BMP180_REG_AC6, &AC6);

	if (!err) {
		err = register_rw(BMP180_REG_B1, &word);
		B1 = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_B2, &word);
		B2 = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_MB, &word);
		MB = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_MC, &word);
		MC = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_MD, &word);
		MD = (int16_t) word;
	}

	return(err);
}

/** Constructor
 */
BMP180::BMP180(uint8_t addr) : address{addr}
{
	uint8_t err;

	// Read the device's id
	err = register_rb(BMP180_REG_ID, &id);

	if (!err && (id == 0x55)) {
		err = register_rb(BMP180_REG_CTRL, &oss);
		oss >>= 6;

		if (!err)
			err = dump_calibration_data();
	}
}

void BMP180::math_temperature()
{
	int32_t x1, x2;

	x1 = (UT - AC6) * AC5 >> 15;
	x2 = ((int32_t)MC << 11) / (x1 + MD);
	B5 = x1 + x2;
	T = (B5 + 8) >> 4;
}

void BMP180::math_pressure()
{
	uint32_t b4, b7;
	int32_t x1, x2, x3, b3, b6;

	b6 = B5 - 4000;
	x1 = (B2 * (b6 * b6 >> 12)) >> 11;
	x2 = AC2 * b6 >> 11;
	x3 = x1 + x2;

	b3 = ((((int32_t)AC1 * 4 + x3) << oss) + 2) >> 2;
	x1 = AC3 * b6 >> 13;
	x2 = (B1 * (b6 * b6 >> 12)) >> 16;
	x3 = (x1 + x2 + 2) >> 2;
	b4 = (AC4 * (uint32_t)(x3 + 32768)) >> 15;
	b7 = (uint32_t)(UP - b3) * (50000 >> oss);

	if (b7 < 0x80000000)
		p = (b7 << 1) / b4;
	else
		p = (b7 / b4) << 1;

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	p += ((x1 + x2 + 3791) >> 4);
}

void BMP180::math_altitude()
{
	altitude = 44330.0F * (1.0F - ((float)pow(((float)p/BMP180_SEALEVEL), 0.190223F)));
}

uint8_t BMP180::resolution(const uint8_t mode)
{
	uint8_t err, byte;

	err = register_rb(BMP180_REG_CTRL, &byte);

	if (!err) {
		byte &= 0x3f; /* 0b00111111 */
		byte |= (mode << 6);
	}

	return(err);
}

/** The temperature read and converter.
 *
 * See datasheet for details.
 */
uint8_t BMP180::read_temperature()
{
	uint8_t err;
	uint16_t word;

	/* Read UT */
	// BMP180_REG_CTRL = 0x2e
	word = (uint16_t)(BMP180_REG_CTRL << 8) | 0x2e;
	err = i2c.tx(WRITE, 2, (uint8_t *) &word);

	if (!err) {
		_delay_ms(5);
		err = register_rw(BMP180_REG_ADC, &word);
		UT = (long)word;

		if (!err)
			math_temperature();
	}

	return (err);
}

uint8_t BMP180::read_pressure()
{
	uint8_t err, byte;
	uint16_t word;

	/* Read UP */
	// BMP180_REG_CTRL = 0x34 + (oss << 6)
	word = (uint16_t)(BMP180_REG_CTRL << 8) | (0x34 + (oss << 6));
	err = i2c.tx(WRITE, 2, (uint8_t *) &word);

	if (!err) {
		switch (oss) {
			case BMP180_RES_LOW:
				_delay_ms(5);
				break;
			case BMP180_RES_STD:
				_delay_ms(8);
				break;
			case BMP180_RES_HIGH:
				_delay_ms(14);
				break;
			default:
				_delay_ms(26);
		}

		err = register_rw(BMP180_REG_ADC, &word);
		UP = (int32_t)word << 8;

		if (!err) {
			err = register_rb(BMP180_REG_ADCXLSB, &byte);
			UP |= byte;
			UP >>= (8 - oss);
		}

		if (!err)
			math_pressure();
	}

	return(err);
}

uint8_t BMP180::read_all()
{
	uint8_t err;

	err = read_temperature();

	if (!err)
		err = read_pressure();

	return(err);
}
