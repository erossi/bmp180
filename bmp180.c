/* Copyright (C) 2013 Enrico Rossi
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
uint8_t register_rb(const uint8_t reg_addr, uint8_t *byte)
{
	uint8_t error;

	error = i2c_master_send_b(BMP180_ADDR, reg_addr, FALSE);

	if (!error)
		error = i2c_master_read_b(BMP180_ADDR, byte, TRUE);

	return (error);
}

/** Register Read (word).
 *
 * @param reg_addr the register address.
 * @param byte the data to be read.
 */
uint8_t register_rw(const uint8_t reg_addr, uint16_t *word)
{
	uint8_t err;

	err = i2c_master_send_b(BMP180_ADDR, reg_addr, FALSE);

	if (!err)
		err = i2c_master_read_w(BMP180_ADDR, word);

	/*
		err = i2c_master_read_w(MPU6050_ADDR, word, TRUE);
		*/

	return (err);
}

/** Register write (Byte).
 *
 * @param reg_addr the register address.
 * @param byte the data to be written.
 */
uint8_t register_wb(const uint8_t reg_addr, uint8_t byte)
{
	uint8_t error;

	error = i2c_master_send_w(BMP180_ADDR, reg_addr, byte);
	return (error);
}

/** Read the calibration data.
 *
 */
uint8_t dump_calibration_data(struct bmp180_t *bmp180)
{
	uint8_t err;
	uint16_t word;

	err = register_rw(BMP180_REG_AC1, &word);
	bmp180->AC1 = (int16_t) word;

	if (!err) {
		err = register_rw(BMP180_REG_AC2, &word);
		bmp180->AC2 = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_AC3, &word);
		bmp180->AC3 = (int16_t) word;
	}

	if (!err)
		err = register_rw(BMP180_REG_AC4, &bmp180->AC4);

	if (!err)
		err = register_rw(BMP180_REG_AC5, &bmp180->AC5);

	if (!err)
		err = register_rw(BMP180_REG_AC6, &bmp180->AC6);

	if (!err) {
		err = register_rw(BMP180_REG_B1, &word);
		bmp180->B1 = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_B2, &word);
		bmp180->B2 = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_MB, &word);
		bmp180->MB = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_MC, &word);
		bmp180->MC = (int16_t) word;
	}

	if (!err) {
		err = register_rw(BMP180_REG_MD, &word);
		bmp180->MD = (int16_t) word;
	}

	return(err);
}

void math_temperature(struct bmp180_t *bmp180)
{
	int32_t x1, x2;

	x1 = (bmp180->UT - bmp180->AC6) * bmp180->AC5 >> 15;
	x2 = ((int32_t)bmp180->MC << 11) / (x1 + bmp180->MD);
	bmp180->B5 = x1 + x2;
	bmp180->T = (bmp180->B5 + 8) >> 4;
}

void math_pressure(struct bmp180_t *bmp180)
{
	uint32_t b4, b7;
	int32_t x1, x2, x3, b3, b6;

	b6 = bmp180->B5 - 4000;
	x1 = (bmp180->B2 * (b6 * b6 >> 12)) >> 11;
	x2 = bmp180->AC2 * b6 >> 11;
	x3 = x1 + x2;

	b3 = ((((int32_t)bmp180->AC1 * 4 + x3) << bmp180->oss) + 2) >> 2;
	x1 = bmp180->AC3 * b6 >> 13;
	x2 = (bmp180->B1 * (b6 * b6 >> 12)) >> 16;
	x3 = (x1 + x2 + 2) >> 2;
	b4 = (bmp180->AC4 * (uint32_t)(x3 + 32768)) >> 15;
	b7 = (uint32_t)(bmp180->UP - b3) * (50000 >> bmp180->oss);

	if (b7 < 0x80000000)
		bmp180->p = (b7 << 1) / b4;
	else
		bmp180->p = (b7 / b4) << 1;

	x1 = (bmp180->p >> 8) * (bmp180->p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * bmp180->p) >> 16;
	bmp180->p += ((x1 + x2 + 3791) >> 4);
}

void bmp180_altitude(struct bmp180_t *bmp180)
{
	bmp180->altitude = 44330.0F * (1.0F - ((float)pow(((float)bmp180->p/(float)bmp180->p0), 0.190223F)));
}

uint8_t bmp180_resolution(const uint8_t mode)
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
uint8_t bmp180_read_temperature(struct bmp180_t *bmp180)
{
	uint8_t err;
	uint16_t word;

	/* Read UT */
	err = register_wb(BMP180_REG_CTRL, 0x2e);

	if (!err) {
		_delay_ms(5);
		err = register_rw(BMP180_REG_ADC, &word);
		bmp180->UT = (long)word;

		if (!err)
			math_temperature(bmp180);
	}

	return (err);
}

uint8_t bmp180_read_pressure(struct bmp180_t *bmp180)
{
	uint8_t err, byte;
	uint16_t word;

	/* Read UP */
	err = register_wb(BMP180_REG_CTRL, (0x34 + (bmp180->oss << 6)));

	if (!err) {
		switch (bmp180->oss) {
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
		bmp180->UP = (int32_t)word << 8;

		if (!err) {
			err = register_rb(BMP180_REG_ADCXLSB, &byte);
			bmp180->UP |= byte;
			bmp180->UP >>= (8 - bmp180->oss);
		}

		if (!err)
			math_pressure(bmp180);
	}

	return(err);
}

/** Init
 */
uint8_t bmp180_init(struct bmp180_t *bmp180)
{
	uint8_t err;

	bmp180->flags = 0;
	bmp180->p0 = BMP180_SEALEVEL;
	i2c_init();
	err = register_rb(BMP180_REG_ID, &bmp180->id);

	if (!err && (bmp180->id == 0x55)) {
		err = register_rb(BMP180_REG_CTRL, &bmp180->oss);
		bmp180->oss >>= 6;

		if (!err)
			err = dump_calibration_data(bmp180);
	}

	return (err);
}

uint8_t bmp180_read_all(struct bmp180_t *bmp180)
{
	uint8_t err;

	err = bmp180_read_temperature(bmp180);

	if (!err)
		err = bmp180_read_pressure(bmp180);

	return(err);
}
