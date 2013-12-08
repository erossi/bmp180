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

/*! \file i2c.h */

#ifndef I2C_DEF
#define I2C_DEF

#define START 1
#define STOP 2
#define SLA 3
#define DATA 4
#define ACK 5
#define NACK 6

#define READ 1
#define WRITE 0
#define TRUE 1
#define FALSE 0

uint8_t i2c_send(const uint8_t code, const uint8_t data);
void i2c_init(void);
uint8_t i2c_master_send_b(const uint8_t addr, const uint8_t data,
		const uint8_t stop);
uint8_t i2c_master_send_w(const uint8_t addr, const uint8_t msb, const uint8_t lsb);
uint8_t i2c_master_read_b(const uint8_t addr, uint8_t *byte,
		const uint8_t stop);
uint8_t i2c_master_read_w(const uint8_t addr, uint16_t *code);

#endif
