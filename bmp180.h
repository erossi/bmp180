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

/*! \file bmp180.h */

#ifndef _BMP180
#define _BMP180

#include <stdint.h>
#include "i2c.h"

#define BMP180_REG_AC1 0xaa
#define BMP180_REG_AC2 0xac
#define BMP180_REG_AC3 0xae
#define BMP180_REG_AC4 0xb0
#define BMP180_REG_AC5 0xb2
#define BMP180_REG_AC6 0xb4
#define BMP180_REG_B1 0xb6
#define BMP180_REG_B2 0xb8
#define BMP180_REG_MB 0xba
#define BMP180_REG_MC 0xbc
#define BMP180_REG_MD 0xbe

#define BMP180_REG_ID 0xd0
#define BMP180_REG_CTRL 0xf4
#define BMP180_REG_ADC 0xf6
#define BMP180_REG_ADCMSB 0xf6
#define BMP180_REG_ADCLSB 0xf7
#define BMP180_REG_ADCXLSB 0xf8

#define BMP180_RES_LOW 0
#define BMP180_RES_STD 1
#define BMP180_RES_HIGH 2
#define BMP180_RES_ULTRAHIGH 3

// C++ compiler
#ifdef __cplusplus

#define BMP180_SEALEVEL 101325.0F // Pressure at sealevel

class BMP180 {
	private:
		const uint8_t address;
		int16_t AC1;
		int16_t AC2;
		int16_t AC3;
		uint16_t AC4;
		uint16_t AC5;
		uint16_t AC6;
		int16_t B1;
		int16_t B2;
		int16_t MB;
		int16_t MC;
		int16_t MD;

		uint8_t oss;

		int32_t UT;
		int32_t UP;
		int32_t T;
		int32_t p;
		// int32_t p0; // BMP180_SEALEVEL

		int32_t B5;
		I2C i2c {address}; // Contructor
		uint8_t register_rb(uint8_t, uint8_t*);
		uint8_t register_rw(uint8_t, uint16_t*);
		uint8_t dump_calibration_data(void);
		void math_temperature();
		void math_pressure();
		void math_altitude();
		uint8_t resolution(const uint8_t);
	public:
		BMP180(uint8_t);
		uint8_t id;
		float altitude;
		uint8_t read_temperature();
		uint8_t read_pressure();
		uint8_t read_all();
};

#else // __cplusplus

/** The address of the device.
 * Typical it is 0x55
 */
#define BMP180_ADDR 0xee
#define BMP180_SEALEVEL 101325L

struct bmp180_t {
	uint8_t id;

	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;

	uint8_t oss;

	int32_t UT;
	int32_t UP;
	int32_t T;
	int32_t p;
	int32_t p0;

	int32_t B5;

	uint8_t flags;
	float altitude;
};

uint8_t bmp180_init(struct bmp180_t *bmp180);
uint8_t bmp180_read_temperature(struct bmp180_t *bmp180);
uint8_t bmp180_read_pressure(struct bmp180_t *bmp180);
uint8_t bmp180_read_all(struct bmp180_t *bmp180);
void bmp180_altitude(struct bmp180_t *bmp180);

#endif // __cplusplus
#endif
