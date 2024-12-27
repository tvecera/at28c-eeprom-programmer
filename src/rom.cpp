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
#include "rom.h"
#include "at28c.h"
#include "util.h"

/**
* @brief Writes ROM data to EEPROM with verification
*
* Process:
*  1. Erases target EEPROM section by setting all bytes to 0xFF
*  2. Writes ROM data from program memory to EEPROM
*  3. Verifies written data against original ROM
*
* @global rom[] - Source ROM data in program memory
* @uses ROM_SIZE - Size of ROM data to write
*/
void eeprom_rom_write() {
	uint32_t errors = 0;
	const unsigned long start_time = millis();

	Serial.println();
	// First, erase the section where ROM will be written
	Serial.println(F("Step 1: Erasing EEPROM section"));
	eeprom_erase_section(0, ROM_SIZE, 0xFF);

	// Write ROM data
	Serial.println(F("\nStep 2: Writing ROM data"));
	for (uint16_t addr = 0; addr < ROM_SIZE; addr++) {
		uint8_t data = pgm_read_byte(&rom[addr]);
		eeprom_write_byte(addr, data);

		print_progress(addr);
	}
	Serial.println(F("\nWrite complete!"));

	// Verify written data
	Serial.println(F("\nStep 3: Verifying ROM data"));
	for (uint16_t addr = 0; addr < ROM_SIZE; addr++) {
		uint8_t expected = pgm_read_byte(&rom[addr]);
		if (!eeprom_verify_byte(addr, expected)) errors++;

		print_progress(addr);
	}

	if (errors == 0) {
		Serial.println(F("\nVerification successful - ROM written correctly!"));
	} else {
		Serial.print(F("\nVerification failed with "));
		Serial.print(errors);
		Serial.println(F(" errors."));
	}

	Serial.println(F("\nROM Writing Complete!"));
	print_execution_time(millis() - start_time);
}
