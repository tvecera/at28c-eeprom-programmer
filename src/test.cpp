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
#include "test.h"
#include "at28c.h"
#include "util.h"

// Walking 1's
#define PATTERN_1 (1 << (addr & 7))
// Address as data
#define PATTERN_2 (addr & 0xFF)
// Alternating 0x55 and 0xAA
#define PATTERN_3 ((addr & 1) ? 0xAA : 0x55)
// All zeros
#define PATTERN_4 0x00
// All ones
#define PATTERN_5 0xFF
// Inverted address
#define PATTERN_6 (~addr & 0xFF)

/**
 * @brief Tests EEPROM memory region by writing and verifying test patterns.
 *
 * This function writes a specified test pattern to each byte in the given EEPROM
 * address range and verifies the written values. It prints progress indicators
 * during testing and reports completion status.
 *
 * @param start   Starting address of EEPROM region to test (inclusive)
 * @param stop    Ending address of EEPROM region to test (exclusive)
 * @param type    Test pattern type (1-6) to write
 * @param name    Name of the test/region being tested (for progress output)
 * @param errors  Pointer to error counter that will be incremented for each
 *                verification failure
 */
static void eeprom_test_pattern(const uint16_t start, const uint16_t stop, const uint8_t type,
                                const __FlashStringHelper *name, uint32_t *errors) {
	Serial.print(F("Testing "));
	Serial.print(name);
	Serial.print(F(" (0x"));
	Serial.print(start, HEX);
	Serial.print(F(" - 0x"));
	Serial.print(stop, HEX);
	Serial.println(F(")"));

	for (uint16_t addr = start; addr < stop; addr++) {
		uint8_t pattern;
		switch (type) {
			case 1: pattern = PATTERN_1;
				break;
			case 2: pattern = PATTERN_2;
				break;
			case 3: pattern = PATTERN_3;
				break;
			case 4: pattern = PATTERN_4;
				break;
			case 5: pattern = PATTERN_5;
				break;
			case 6: pattern = PATTERN_6;
				break;
			default: pattern = PATTERN_1;
				break;
		}
		eeprom_write_byte(addr, pattern);

		if (!eeprom_verify_byte(addr, pattern)) *errors = *errors + 1;

		print_progress(addr - start);
	}

	Serial.println();
	if (*errors != 0) {
		Serial.print(F("Test failed with "));
		Serial.print(*errors);
		Serial.println(F(" errors.\n"));
	} else {
		Serial.print(F("Testing "));
		Serial.print(name);
		Serial.println(F(" - Done.\n"));
	}
}

/**
 * @brief Performs a test of the entire EEPROM memory using multiple test patterns.
 * This function divides the EEPROM into 6 equal segments and tests each segment with a different pattern:
 * - Segment 1: Walking 1's pattern
 * - Segment 2: Address as data pattern
 * - Segment 3: Alternating 0x55/0xAA pattern
 * - Segment 4: All zeros pattern
 * - Segment 5: All ones pattern
 * - Segment 6: Inverted address pattern
 *
 * @see eeprom_test_pattern() Used internally to test each segment
 * @see print_execution_time() Used to report the test duration
 */
void eeprom_test() {
	const unsigned long start_time = millis();
	uint32_t errors = 0;

	Serial.println();
	Serial.println(F("Starting Full EEPROM Test"));
	Serial.print(F("Testing "));
	Serial.print(EEPROM_SIZE);
	Serial.println(F(" bytes"));

	// Calculate segment size for testing
	// 6 different test patterns
	constexpr uint16_t segment_size = EEPROM_SIZE / 6;

	// Test pattern 1: Walking 1's (first segment)
	eeprom_test_pattern(0, segment_size, 1, F("Pattern 1: Walking 1's"), &errors);

	// Test pattern 2: Address as data (second segment)
	eeprom_test_pattern(segment_size, segment_size * 2, 2, F("Pattern 2: Address as data"), &errors);

	// Test pattern 3: Alternating 0x55/0xAA (third segment)
	eeprom_test_pattern(segment_size * 2, segment_size * 3, 3, F("Pattern 3: Alternating 0x55/0xAA"), &errors);

	// Test pattern 4: All zeros (fourth segment)
	eeprom_test_pattern(segment_size * 3, segment_size * 4, 4, F("Pattern 4: All zeros"), &errors);

	// Test pattern 5: All ones (fifth segment)
	eeprom_test_pattern(segment_size * 4, segment_size * 5, 5, F("Pattern 5: All ones"), &errors);

	// Test pattern 6: Inverted address (last segment)
	eeprom_test_pattern(segment_size * 5, EEPROM_SIZE, 6, F("Pattern 6: Inverted address"), &errors);

	// Print final results
	Serial.println(F("EEPROM Test Complete"));
	Serial.print(F("Tested "));
	Serial.print(EEPROM_SIZE);
	Serial.println(F(" bytes\n"));

	if (errors == 0) {
		Serial.println(F("EEPROM test passed successfully!"));
	} else {
		Serial.print(F("Test failed with "));
		Serial.print(errors);
		Serial.println(F(" errors."));
	}

	print_execution_time(millis() - start_time);
}
