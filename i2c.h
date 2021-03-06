/* Copyright (C) 2011-2017 Enrico Rossi
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

#include <stdint.h>

/* common defs */
#define I2C_GC_RESET 0
#define I2C_TIMEOUT 0xff

#define READ 1
#define WRITE 0

// C++ compiler
#ifdef __cplusplus

class I2C {
	private:
		// I2C bus should be initialized only once
		static bool initialized; // class attribute
		static uint8_t bus_status;
		uint8_t error;
		const uint8_t address; // device's address
		void send(const uint8_t, const uint8_t);
	public:
		I2C(uint8_t); // set the device address
		static void Init(); // Initialize bus
		static void Shut(); // De-initialize bus
		uint8_t tx(bool, const uint16_t, uint8_t*, bool = true);
		uint8_t gc(const uint8_t);
};

#else /* __cplusplus */
#include <stdint.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

uint8_t i2c_send(const uint8_t code, const uint8_t data);
void i2c_init(void);
void i2c_shut(void);
uint8_t i2c_mtm(const uint8_t addr, const uint16_t lenght,
		uint8_t *data, const uint8_t stop);
uint8_t i2c_mrm(const uint8_t addr, const uint16_t lenght,
		uint8_t *data, const uint8_t stop);
uint8_t i2c_gc(const uint8_t call);

#ifdef I2C_LEGACY_MODE
uint8_t i2c_master_send_b(const uint8_t addr, const uint8_t data,
		const uint8_t stop);
uint8_t i2c_master_send_w(const uint8_t addr, const uint8_t msb,
		const uint8_t lsb);
uint8_t i2c_master_read_b(const uint8_t addr, uint8_t *byte,
		const uint8_t stop);
uint8_t i2c_master_read_w(const uint8_t addr, uint16_t *data);
#endif /* I2C_LEGACY_MODE */
#endif /* CPP */
#endif
