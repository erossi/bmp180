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

/*! \file uart.h
 * \brief RS232 - IO functions.
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

/*! UART 0 baud rate */
#define UART_BAUD_0 9600
/*! UART 1 baud rate */
#define UART_BAUD_1 9600
/*! IO Buffers and masks */
#define UART_RXBUF_SIZE 64
/*! IO Buffers and masks */
#define UART_TXBUF_SIZE 64
/*! IO Buffers and masks */
#define UART_RXBUF_MASK ( UART_RXBUF_SIZE - 1 )
/*! IO Buffers and masks */
#define UART_TXBUF_MASK ( UART_TXBUF_SIZE - 1 )
/*! Check if something is wrong in the definitions */
#if ( UART_RXBUF_SIZE & UART_RXBUF_MASK )
#error RX buffer size is not a power of 2
#endif
/*! Check if something is wrong in the definitions */
#if ( UART_TXBUF_SIZE & UART_TXBUF_MASK )
#error TX buffer size is not a power of 2
#endif

/*! Structure with IO buffers and indexes */
struct uartStruct {
	/*! receive buffer. */
        char *rx_buffer;
	/*! transmit buffer. */
        char *tx_buffer;
	/*! flags. */
        volatile uint8_t rx_flag;
	/*! flags. */
        volatile uint8_t tx_flag;
	/*! flags. */
        volatile uint8_t rxIdx;
	/*! flags. */
        volatile uint8_t txIdx;
};

void uart_init(const uint8_t port);
void uart_shutdown(const uint8_t port);
char uart_getchar(const uint8_t port, const uint8_t locked);
void uart_putchar(const uint8_t port, const char c);
void uart_printstr(const uint8_t port, const char *s);

#endif
