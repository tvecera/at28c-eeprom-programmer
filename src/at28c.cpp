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

#include <Wire.h>
#include "main.h"
#include "at28c.h"
#include "util.h"

#define MCP23017_ADDRESS  0x20   // Default I2C address (A0-A2 grounded)
#define MCP23017_IODIRA   0x00   // I/O direction register for port A
#define MCP23017_IODIRB   0x01   // I/O direction register for port B
#define MCP23017_GPIOA    0x12   // Port register for port A
#define MCP23017_GPIOB    0x13   // Port register for port B
#define MCP23017_PORTA    0
#define MCP23017_PORTB    1

#define tAS               1   // tAS (Address Setup Time) = 10 ns minimum
#define tWP               1   // tWP (Write Pulse Width) = 100 ns minimum, 1000 ns maximum
#define tDH               1   // tDH (Data Hold Time) = 10 ns minimum

#define ce0() digitalWrite(CE_PIN, LOW)
#define ce1() digitalWrite(CE_PIN, HIGH)
#define we0() digitalWrite(WE_PIN, LOW)
#define we1() digitalWrite(WE_PIN, HIGH)
#define oe0() digitalWrite(OE_PIN, LOW)
#define oe1() digitalWrite(OE_PIN, HIGH)

inline void set_address(uint16_t address) __attribute__((always_inline));

inline void set_port_mode(uint8_t port, uint8_t mode) __attribute__((always_inline));

inline void write_data(uint8_t data) __attribute__((always_inline));

inline uint8_t read_data() __attribute__((always_inline));

/**
 * @brief Set the address for the EEPROM. This function sets the lower 8 bits (A0-A7) using MCP23017 PORTB.
 * The upper bits (A8-A12) are set using Arduino pins.
 * @param address EEPROM address
 */
inline void set_address(uint16_t address) {
	Wire.beginTransmission(MCP23017_ADDRESS);
	Wire.write(MCP23017_GPIOB);
	Wire.write(address & 0xFF);
	Wire.endTransmission();

	// Set upper address bits using Arduino pins
	digitalWrite(A8_PIN, (address >> 8) & 1);
	digitalWrite(A9_PIN, (address >> 9) & 1);
	digitalWrite(A10_PIN, (address >> 10) & 1);
	digitalWrite(A11_PIN, (address >> 11) & 1);
	digitalWrite(A12_PIN, (address >> 12) & 1);

#if CHIP_TYPE == 256
    digitalWrite(A13_PIN, (address >> 13) & 1);
    digitalWrite(A14_PIN, (address >> 14) & 1);
#endif
}

/**
 * @brief Set the I/O direction for a given MCP23017 port.
 * @param port Port number (PORTA = 0 or PORTB = 1)
 * @param mode INPUT or OUTPUT
 */
inline void set_port_mode(const uint8_t port, const uint8_t mode) {
	Wire.beginTransmission(MCP23017_ADDRESS);
	Wire.write(port == 0 ? MCP23017_IODIRA : MCP23017_IODIRB);
	Wire.write(mode == INPUT ? 0xFF : 0x00); // All pins INPUT or OUTPUT
	Wire.endTransmission();
}

/**
 * @brief Write data to MCP23017 GPIOA register - PORTA.
 * @param data Data to write
 */
inline void write_data(const uint8_t data) {
	Wire.beginTransmission(MCP23017_ADDRESS);
	Wire.write(MCP23017_GPIOA);
	Wire.write(data & 0xFF);
	Wire.endTransmission();
}

/**
 * @brief Read data from MCP23017 GPIOA register - PORTA.
 * @return Byte containing all 8 pins
 */
inline uint8_t read_data() {
	Wire.beginTransmission(MCP23017_ADDRESS);
	Wire.write(MCP23017_GPIOA); // Read from GPIOA register
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS, 1);
	return Wire.read(); // Returns the byte containing all 8 pins
}

/**
 * @brief Send a command to the EEPROM. Chip select, or other flags are not managed here!!!
 * @param address EEPROM address
 * @param command Command to send
 */
static void send_command(const uint16_t address, const uint8_t command) {
	set_address(address);
	write_data(command);
	delayMicroseconds(tAS); // tAS: Address setup time
	we0();
	delayMicroseconds(tWP); // tWP: Write pulse width
	we1();
	delayMicroseconds(tDH); // tDH: Data hold time
}

/**
 * @brief Enable or disable EEPROM write protection. This function is not available on all AT28C EEPROMs.
 * @param enable Enable write protection
 */
void eeprom_write_protect(const bool enable) {
	oe1();
	we1();
	ce0();
	set_port_mode(MCP23017_PORTA, OUTPUT);

#if CHIP_TYPE == 64
	if (enable) {
		// Enable Software Data Protection (SDP)
		send_command(0x1555, 0xAA);
		send_command(0x0AAA, 0x55);
		send_command(0x1555, 0xA0);
	} else {
		// Disable Software Data Protection (SDP)
		send_command(0x1555, 0xAA);
		send_command(0x0AAA, 0x55);
		send_command(0x1555, 0x80);
		send_command(0x1555, 0xAA);
		send_command(0x0AAA, 0x55);
		send_command(0x1555, 0x20);
	}
#endif

#if CHIP_TYPE == 256
  if (enable) {
    // Enable Software Data Protection (SDP)
    send_command(0x5555, 0xAA);
    send_command(0x2AAA, 0x55);
    send_command(0x5555, 0xA0);
  } else {
    // Disable Software Data Protection (SDP)
    send_command(0x5555, 0xAA);
    send_command(0x2AAA, 0x55);
    send_command(0x5555, 0x80);
    send_command(0x5555, 0xAA);
    send_command(0x2AAA, 0x55);
    send_command(0x5555, 0x20);
  }
#endif

	delay(10);
	ce1();
	set_port_mode(MCP23017_PORTA, INPUT);
}

/**
 * @brief Initialize the EEPROM programmer. This function must be called before any other EEPROM functions.
 * @return True if initialization was successful, false otherwise
 */
bool eeprom_init() {
	Wire.begin();

	we1();
	oe1();
	ce1();
	pinMode(WE_PIN, OUTPUT);
	pinMode(OE_PIN, OUTPUT);
	pinMode(CE_PIN, OUTPUT);

	pinMode(A8_PIN, OUTPUT);
	pinMode(A9_PIN, OUTPUT);
	pinMode(A10_PIN, OUTPUT);
	pinMode(A11_PIN, OUTPUT);
	pinMode(A12_PIN, OUTPUT);
#if CHIP_TYPE == 256
    pinMode(A13_PIN, OUTPUT);
    pinMode(A14_PIN, OUTPUT);
#endif

	// Configure MCP23017 ports
	set_port_mode(MCP23017_PORTA, INPUT);
	set_port_mode(MCP23017_PORTB, OUTPUT);

	return true;
}

/**
 * @brief Write a byte to the EEPROM and use data polling to verify the write operation.
 * @param address EEPROM address
 * @param data Data to write
 */
void eeprom_write_byte(const uint16_t address, const uint8_t data) {
	if (address >= EEPROM_SIZE) return;

	// Prepare for write
	oe1();
	we1();
	set_address(address);
	set_port_mode(MCP23017_PORTA, OUTPUT);
	write_data(data);

	// Begin write cycle
	ce0();
	delayMicroseconds(tAS); // Address setup time
	we0();
	delayMicroseconds(tWP); // Write pulse width
	we1();
	delayMicroseconds(tDH); // Data hold time
	ce1();

	// Switch to reading for data polling
	set_port_mode(MCP23017_PORTA, INPUT);
	we1();
	ce0();
	oe0();

	// Poll I/O7 until write completes
	// During write, I/O7 outputs complement of written data
	// Write is complete when I/O7 matches written data
	const uint8_t expected_io7 = data & 0x80;
	uint8_t current_io7;

	do {
		current_io7 = read_data() & 0x80;
	} while (current_io7 != expected_io7);

	oe1();
	ce1();
}

/**
 * @brief Read a byte from the EEPROM.
 * @param address EEPROM address
 * @return Data read from the EEPROM
 */
uint8_t eeprom_read_byte(const uint16_t address) {
	if (address >= EEPROM_SIZE) return 0xFF;

	set_address(address);
	set_port_mode(MCP23017_PORTA, INPUT);
	oe1();
	we1();
	ce0();
	delayMicroseconds(tAS); // tAS: Address setup time
	oe0();
	delayMicroseconds(100); // tOE: Output enable time
	const uint8_t data = read_data();
	oe1();
	ce1();

	return data;
}

/**
 * @brief Verify a byte in the EEPROM against an expected value.
 * @param address EEPROM address
 * @param expected Expected data
 * @return True if the data matches, false otherwise
 */
bool eeprom_verify_byte(const uint16_t address, const uint8_t expected) {
	const uint8_t read_back = eeprom_read_byte(address);

	if (read_back != expected) {
		Serial.print(F("\nVerification failed at 0x"));
		Serial.print(address, HEX);
		Serial.print(F(": Expected 0x"));
		Serial.print(expected, HEX);
		Serial.print(F(", Read 0x"));
		Serial.println(read_back, HEX);
		return false;
	}

	return true;
}

/**
 * @brief Erase a section of the EEPROM by writing a pattern to all bytes.
 * @param start Start address
 * @param end End address
 * @param pattern Pattern to write
 */
void eeprom_erase_section(const uint16_t start, const uint16_t end, const uint8_t pattern) {
	Serial.print(F("Erasing EEPROM from 0x"));
	Serial.print(start, HEX);
	Serial.print(F(" to 0x"));
	Serial.print(end - 1, HEX);
	Serial.print(F(" with pattern 0x"));
	Serial.println(pattern, HEX);

	for (uint16_t addr = start; addr < end; addr++) {
		eeprom_write_byte(addr, pattern);

		print_progress(addr);
	}

	Serial.println(F("\nErase Done!"));
}
