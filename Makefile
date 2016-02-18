# Copyright (C) 2013 Enrico Rossi
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

PRG_NAME = BMP180
MCU = atmega328p
OPTLEV = s
FCPU = 16000000UL
PWD = $(shell pwd)
INC = -I/usr/lib/avr/include/

CFLAGS = $(INC) -Wall -Wstrict-prototypes -pedantic -mmcu=$(MCU) -O$(OPTLEV) -D F_CPU=$(FCPU)
LFLAGS = -lm

PRGNAME = $(PRG_NAME)
GIT_TAG = "Unknown"
# Uncomment if git tag is in use
#GIT_TAG = "$(shell git describe --tags)"
#PRGNAME = $(PRG_NAME)_$(GIT_TAG)_$(MCU)

AR = avr-ar
CC = avr-gcc

# Arduino
DUDEAPORT = /dev/ttyACM0
DUDEADEV = arduino

# Stk500v2
DUDESPORT = /dev/ttyUSB0
DUDESDEV = stk500v2

# avrispmkII
DUDEUPORT = usb
DUDEUDEV = avrispmkII

# Use sudo for USB avrispmkII
DUDE = sudo avrdude -p $(MCU) -e -U flash:w:$(PRGNAME).hex
DUDE = avrdude -p $(MCU) -e -U flash:w:$(PRGNAME).hex

OBJCOPY = avr-objcopy -j .text -j .data -O ihex
OBJDUMP = avr-objdump
SIZE = avr-size --format=avr --mcu=$(MCU) $(PRGNAME).elf

REMOVE = rm -f

CFLAGS += -D I2C_LEGACY_MODE
objects = uart.o i2c.o bmp180.o

.PHONY: clean indent
.SILENT: help
.SUFFIXES: .c, .o

# Export variables used in sub-make
.EXPORT_ALL_VARIABLES: doc

all: $(objects)
	$(CC) $(CFLAGS) -o $(PRGNAME).elf main.c $(objects) $(LFLAGS)
	$(OBJCOPY) $(PRGNAME).elf $(PRGNAME).hex

debug.o:
	$(CC) $(CFLAGS) -D GITREL=\"$(GIT_TAG)\" -c debug.c

programardu:
	$(DUDE) -c $(DUDEADEV) -P $(DUDEAPORT)

arduino: programardu

programstk:
	$(DUDE) -c $(DUDESDEV) -P $(DUDESPORT)

program:
	$(DUDE) -c $(DUDEUDEV) -P $(DUDEUPORT)

clean:
	$(REMOVE) *.elf *.hex $(objects)

version:
	# Last Git tag: $(GIT_TAG)

doc_timing:
	$(MAKE) -C ../doc timing

doc:
	$(MAKE) -C ../doc doc

size:
	$(SIZE)
