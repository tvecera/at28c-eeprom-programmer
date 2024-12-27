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

#include "main.h"
#include "at28c.h"
#include "intel_hex.h"
#include "rom.h"
#include "test.h"
#include "util.h"

void print_help() {
	Serial.println();
	Serial.println(F("Commands:"));
	Serial.println(F(" E - Erase EEPROM"));
	Serial.println(F(" T - Full EEPROM test"));
	Serial.println(F(" D - Dump EEPROM contents"));
	Serial.println(F(" W - Write Intel HEX data to EEPROM"));
	Serial.println(F(" R - Write default ROM data from UNO flash"));
	Serial.println(F(" X - Enable write protection"));
	Serial.println(F(" S - Disable write protection"));
	Serial.println(F(" ? - Help"));
	Serial.println();
	Serial.print(F(">"));
}

void setup() {
	Serial.begin(115200);
	while (!Serial) delay(10);

	Serial.println();
	Serial.println(F("======================"));
	Serial.println(F("EEPROM Programmer v0.1 "));
	Serial.println(F("======================"));
	Serial.print(F("\nSelected chip: "));
	Serial.println(F(CHIP_NAME));
	Serial.print(F("Memory size:   "));
	Serial.println(EEPROM_SIZE);

	// Initialize MCP23017
	if (!eeprom_init()) {
		Serial.println(F("Error: MCP23017 initialization failed!"));
		while (1);
	}
	print_help();
}

void dump_eeprom() {
	// Get start address from user
	Serial.println();
	Serial.print(F("Addr: "));
	const uint16_t start_addr = get_hex_value(4, 0) % EEPROM_SIZE;

	uint8_t linesOnPage = 0;

	for (uint32_t addr = start_addr; addr < EEPROM_SIZE; addr++) {
		// Print address at start of line
		if (addr % 16 == 0) {
			Serial.println();
			linesOnPage++;

			// Check if we've printed 10 lines
			if (linesOnPage >= 10) {
				Serial.print(F("Press SPACE to continue, Q to quit..."));
				// Wait for input
				while (true) {
					if (Serial.available()) {
						const char input = static_cast<char>(toupper(Serial.read()));
						if (input == 'Q') {
							Serial.println();
							return; // Exit if space was pressed
						}
						if (input == ' ') {
							Serial.println();
							linesOnPage = 0; // Reset line counter
							while (Serial.available()) Serial.read(); // Clear input buffer
							break;
						}
					}
				}
			}

			// Print address
			if (addr < 0x1000) Serial.print('0');
			if (addr < 0x100) Serial.print('0');
			if (addr < 0x10) Serial.print('0');
			Serial.print(addr, HEX);
			Serial.print(F(": "));
		}

		// Print data byte
		const uint8_t data = eeprom_read_byte(addr);
		if (data < 0x10) Serial.print('0');
		Serial.print(data, HEX);
		Serial.print(' ');

		// Add delay every 256 bytes to allow serial buffer to clear
		if ((addr & 0xFF) == 0xFF) {
			delay(100);
		}
	}
}

void erase_eeprom() {
	Serial.println();
	Serial.print(F("Start: "));
	const uint16_t start = get_hex_value(4, 0) % EEPROM_SIZE;
	Serial.println();
	Serial.print(F("End: "));
	const uint16_t end = get_hex_value(4, 0) % EEPROM_SIZE;
	Serial.println();
	Serial.print(F("Pattern: "));
	const uint8_t pattern = get_hex_value(2, 0xFF);
	Serial.println();

	const unsigned long start_time = millis();
	eeprom_erase_section(start, end == 0x0000 ? EEPROM_SIZE : end, pattern);
	print_execution_time(millis() - start_time);
}

void hex_write() {
	Serial.println();
	bool run = true;
	delay(200);
	while (Serial.available()) Serial.read();

	Serial.println(F("Enter Intel HEX data (finish with empty line):"));
	hex_process_reset();

	while (run) {
		if (Serial.available()) {
			const char c = static_cast<char>(Serial.read());
			const uint8_t result = hex_process_char(c);
			if (result == 2) {
				run = false;
			}
		}
	}
}

void check() {
	Serial.println("\nChecking EEPROM contents...");
	for (uint16_t i = 0; i < ROM_SIZE; i++) {
		uint8_t data = pgm_read_byte(&rom[i]);
		uint8_t readback = eeprom_read_byte(i);
		if (readback != data) {
			Serial.print(F("Verification failed at 0x"));
			Serial.print(i, HEX);
			Serial.print(F(": wrote 0x"));
			Serial.print(data, HEX);
			Serial.print(F(", read 0x"));
			Serial.println(readback, HEX);
		}
	}
	Serial.println(F("Check complete!"));
}

void write_protect(const bool enable) {
	Serial.println();
	Serial.print(F("Write protection: "));
	if (enable) {
		Serial.println(F("enable..."));
	} else {
		Serial.println(F("disable..."));
	}
	eeprom_write_protect(enable);
	Serial.println(F("Done."));
}

void loop() {
	if (Serial.available()) {
		const char c = static_cast<char>(Serial.read());

		if (c == '\r' || c == '\n') {
			Serial.print(F("\n>"));
			delay(100);
			while (Serial.available()) Serial.read();
			return;
		}
		switch (toupper(c)) {
			case 'E':
				Serial.print(F("E"));
				erase_eeprom();
				Serial.flush();
				print_help();
				break;

			case 'C':
				Serial.print(F("C"));
				check();
				Serial.flush();
				print_help();
				break;

			case 'D':
				Serial.print(F("D"));
				dump_eeprom();
				Serial.flush();
				print_help();
				break;

			case 'W':
				Serial.print(F("W"));
				hex_write();
				print_help();
				break;

			case 'R':
				Serial.print(F("R"));
				eeprom_rom_write();
				Serial.flush();
				print_help();
				break;

			case 'T':
				Serial.print(F("T"));
				eeprom_test();
				Serial.flush();
				print_help();
				break;

			case 'X':
				Serial.print(F("X"));
				write_protect(true);
				Serial.flush();
				print_help();
				break;

			case 'S':
				Serial.print(F("S"));
				write_protect(false);
				Serial.flush();
				print_help();
				break;

			case '?':
				Serial.print(F("?"));
				print_help();
				break;

			case '\r':
			case '\n':
				break;

			default:
				Serial.println(F("\nUnknown command. Type ? for help."));
				Serial.print(F(">"));
				break;
		}
	}
}
