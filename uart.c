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

#include <stdint.h>
#include <avr/io.h>
#include "uart.h"

/*! Init the uart port. */
void uart_init(const uint8_t port)
{
	/* Fixed 115200 bps value */
	UBRR0L = 8;
	/*! tx/rx enable */
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
	/* 8n2 */
	UCSR0C = _BV(USBS0) | _BV(UCSZ00) | _BV(UCSZ01);
}

/*! Disable the uart port. */
void uart_shutdown(const uint8_t port)
{
	UCSR0C = 0;
	UCSR0B = 0;
	UBRR0L = 0;
	UCSR0A = 0;
}

/*! Get a char from the uart port. */
char uart_getchar(const uint8_t port, const uint8_t locked)
{
	if (locked) {
		loop_until_bit_is_set(UCSR0A, RXC0);
		return(UDR0);
	} else {
		if (bit_is_set(UCSR0A, RXC0))
			return(UDR0);
		else
			return(0);

	}
}

/*! Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
void uart_putchar(const uint8_t port, const char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

/*! Send a C (NUL-terminated) string down the UART Tx.
 */
void uart_printstr(const uint8_t port, const char *s)
{
	while (*s) {
		if (*s == '\n')
			uart_putchar (0, '\r');

		uart_putchar(port, *s++);
	}
}
