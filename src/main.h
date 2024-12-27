/*
 * MIT License
 *
 * Copyright (c) 2024 Tomas Vecera, tomas@vecera.dev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef EEPROM_MAIN_H
#define EEPROM_MAIN_H

#include <Arduino.h>

/*
 * Select your chip type:
 * - 64 for AT28C64
 * - 256 for AT28C256
 *            _____   _____
 *   RDY   1 |*    \_/     | 28  VCC
 *   A12   2 |             | 27  WE
 *    A7   3 |             | 26  NC
 *    A6   4 |   AT28C64   | 25  A8
 *    A5   5 |             | 24  A9
 *    A4   6 |             | 23  A11
 *    A3   7 |             | 22  OE
 *    A2   8 |             | 21  A10
 *    A1   9 |             | 20  CE
 *    A0  10 |             | 19  I/O7
 *  I/O0  11 |             | 18  I/O6
 *  I/O1  12 |             | 17  I/O5
 *  I/O2  13 |             | 16  I/O4
 *   GND  14 |_____________| 15  I/O3
 */
#define CHIP_TYPE   64

/*
 * Arduino to MCP23017:
 * - A4 -> SDA
 * - A5 -> SCL
 * - 5V -> VDD
 * - GND -> VSS
 * - MCP23017 A0,A1,A2 -> GND (I2C address 0x20)
 *
 * Arduino to AT28C64:
 * - Pin 2 -> WE (Pin 27)
 * - Pin 3 -> OE (Pin 22)
 * - Pin 4 -> CE (Pin 20)
 * - Pin 5 -> A8 (Pin 25)
 * - Pin 6 -> A9 (Pin 24)
 * - Pin 7 -> A10 (Pin 21)
 * - Pin 8 -> A11 (Pin 23)
 * - Pin 9 -> A12 (Pin 2)
 * - Pin 10 -> A13 (Pin 1) (AT28C256 only)
 * - Pin 11 -> A14 (Pin 31) (AT28C256 only)
 *
 * MCP23017 to AT28C64:
 * PORTA:
 * - PA0 -> D0 (Pin 11)
 * - PA1 -> D1 (Pin 12)
 * - PA2 -> D2 (Pin 13)
 * - PA3 -> D3 (Pin 15)
 * - PA4 -> D4 (Pin 16)
 * - PA5 -> D5 (Pin 17)
 * - PA6 -> D6 (Pin 18)
 * - PA7 -> D7 (Pin 19)
 *
 * PORTB:
 * - PB0 -> A0 (Pin 10)
 * - PB1 -> A1 (Pin 9)
 * - PB2 -> A2 (Pin 8)
 * - PB3 -> A3 (Pin 7)
 * - PB4 -> A4 (Pin 6)
 * - PB5 -> A5 (Pin 5)
 * - PB6 -> A6 (Pin 4)
 * - PB7 -> A7 (Pin 3)
 */

// Arduino pin assignments for address lines
#define A8_PIN      5    // Address line A8
#define A9_PIN      6    // Address line A9
#define A10_PIN     7    // Address line A10
#define A11_PIN     8    // Address line A11
#define A12_PIN     9    // Address line A12
#define A13_PIN     10   // Address line A13 (AT28C256 only)
#define A14_PIN     11   // Address line A14 (AT28C256 only)

// Control pins
#define WE_PIN      2    // Write Enable (active low)
#define OE_PIN      3    // Output Enable (active low)
#define CE_PIN      4    // Chip Enable (active low)

#endif //EEPROM_MAIN_H
