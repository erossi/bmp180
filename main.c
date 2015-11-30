/* Copyright (C) 2013, 2015 Enrico Rossi
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
#include "uart.h"

/*! Print the bmp180 struct content
 *
 */
void print_struct(struct bmp180_t *bmp180, char *string)
{
	string = utoa(bmp180->id, string, 16);
	uart_printstr(0, "id: 0x");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = utoa(bmp180->oss, string, 16);
	uart_printstr(0, "oss: 0x");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->AC1, string, 10);
	uart_printstr(0, "AC1: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->AC2, string, 10);
	uart_printstr(0, "AC2: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->AC3, string, 10);
	uart_printstr(0, "AC3: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = utoa(bmp180->AC4, string, 10);
	uart_printstr(0, "AC4: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = utoa(bmp180->AC5, string, 10);
	uart_printstr(0, "AC5: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = utoa(bmp180->AC6, string, 10);
	uart_printstr(0, "AC6: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->B1, string, 10);
	uart_printstr(0, "B1: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->B2, string, 10);
	uart_printstr(0, "B2: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->MB, string, 10);
	uart_printstr(0, "MB: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->MC, string, 10);
	uart_printstr(0, "MC: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = itoa(bmp180->MD, string, 10);
	uart_printstr(0, "MD: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");
}

void print_error(uint8_t error, char *string)
{
	string = utoa(error, string, 16);
	uart_printstr(0, "\nError: 0x");
	uart_printstr(0, string);
	uart_printstr(0, "\n");
}

/*! Print the temperature and pressure.
 *
 * Both the real value and the uncalibrated one.
 */
void print_results(struct bmp180_t *bmp180, char *string)
{
	string = ultoa(bmp180->UT, string, 10);
	uart_printstr(0, "UT: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = ultoa(bmp180->UP, string, 10);
	uart_printstr(0, "UP: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = ultoa(bmp180->T, string, 10);
	uart_printstr(0, "T: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");

	string = ultoa(bmp180->p, string, 10);
	uart_printstr(0, "p: ");
	uart_printstr(0, string);
	uart_printstr(0, "\n");
}

/* Simulate the sound of a buzzer.
 *
 * Print what will happen when the buzzer will be connected
 * beeping up and down different tone depending on how fast (m/s)
 * the sensor is moving.
 */
void beep(int32_t dp, const char updown, char *string)
{
	int8_t beeps;
	uint8_t i;

	beeps = (int8_t)(dp/12);
	string = itoa(beeps, string, 10);

	if (updown == '+') {
		uart_printstr(0, "beeps UP ");

		for (i=0; i<50; i++) {
			PORTC |= _BV(PC0);
			_delay_us(100);
			PORTC = 0;
			_delay_us(900);
		}
	} else {
		uart_printstr(0, "beeps DOWN ");
	}

	uart_printstr(0, string);
	uart_printstr(0, " ");
}

int main(void)
{
	struct bmp180_t *bmp180;
	char *string;
	uint8_t i, err;
	int32_t pmed, pold, dp;
	float dA;

	PORTC = 0;
	DDRC |= _BV(PC0);

	string = malloc(80);
	bmp180 = malloc(sizeof(struct bmp180_t));

	uart_init(0);
	uart_printstr(0, "BMP180 example prg.\n");

	_delay_ms(1000);

	err = bmp180_init(bmp180);

	if (err)
		print_error(err, string);

	print_struct(bmp180, string);
	bmp180->oss = BMP180_RES_ULTRAHIGH;
	err = bmp180_read_all(bmp180);
	pold = bmp180->p;
	dA = 0;

	if (!err)
		print_results(bmp180, string);

	/* try media for 32 readings,
	 * p = ((p*31) + new)/2^6)
	 */
	while(1) {
		i = 0;
		pmed = 0;

		/* use and average of 32 readings */
		while (i<32) {
			err = bmp180_read_pressure(bmp180);
			pmed += bmp180->p;
			i++;
		}

		/* pmed = pmed / 32 */
		pmed >>= 5;
		dp = pmed-pold;

		/* Dp (delta pressure) of 1hpa = 8.43m @ sea level */
		dA = (dA + dp * -8.43) / 2;
		pold = pmed;

		if (dp > 12)
			beep(dp, '-', string);
		else if (dp < -12)
			beep(dp, '+', string);

		string = ultoa(pmed, string, 10);
		uart_printstr(0, string);
		uart_printstr(0, " ");

		string = itoa(dp, string, 10);
		uart_printstr(0, string);
		uart_printstr(0, " ");

		string = dtostrf(dA, 10, 2, string);
		uart_printstr(0, string);
		uart_printstr(0, "\n");
	}

	return(0);
}
