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
#include "util.h"

#define MAX_LINE_LENGTH 45

static char input_buffer[MAX_LINE_LENGTH + 1];
int input_index = 0;

/**
 * @brief Processes a single line of Intel HEX format data and writes it to EEPROM
 *
 * This function parses and processes a line of Intel HEX format, performing the following:
 *	- Validates line length and start character (':')
 *	- Extracts and converts hex values for byte count, address, and record type
 *	- Handles different record types:
 *		- 0x00: Data record (writes data to EEPROM with verification)
 *		- 0x01: End of file record
 *		- Others: Treated as unsupported
 *
 * Format: :BBAAAATTDD...CC where:
 *  - BB: Byte count
 *  - AAAA: Address
 *  - TT: Record type
 *  - DD: Data bytes
 *  - CC: Checksum (not currently verified)
 *
 * @param line Pointer to null-terminated string containing a single Intel HEX format line
 * @return uint8_t Status code:
 *         - 0 (false): Error occurred during processing
 *         - 1 (true): Line processed successfully
 *         - 2: End of file record processed successfully
 */
static uint8_t hex_process_line(const char *line) {
	if (strlen(line) < 11) {
		Serial.println(F("Error: Line too short"));
		return false;
	}

	// Check start character
	if (line[0] != ':') {
		Serial.println(F("Error: Missing start character (:)"));
		return false;
	}

	// Convert hex values
	const uint8_t byte_count = (hex_char_to_int(line[1]) << 4) | hex_char_to_int(line[2]);
	const uint16_t address = (hex_char_to_int(line[3]) << 12) | (hex_char_to_int(line[4]) << 8) |
	                         (hex_char_to_int(line[5]) << 4) | hex_char_to_int(line[6]);
	const uint8_t record_type = (hex_char_to_int(line[7]) << 4) | hex_char_to_int(line[8]);

	Serial.print(F("Line - Type: "));
	Serial.print(record_type, HEX);
	if (record_type == 0x00) {
		Serial.print(F(", Address: "));
		Serial.print(address, HEX);
		Serial.print(F("h, Byte count: "));
		Serial.println(byte_count);
	}

	// Handle record types
	switch (record_type) {
		case 0x00: // Data record
			// Write data to EEPROM
			for (uint8_t i = 0; i < byte_count; i++) {
				const uint8_t data = (hex_char_to_int(line[9 + i * 2]) << 4) | hex_char_to_int(line[10 + i * 2]);
				eeprom_write_byte(address + i, data);

				// Optional: Add verification
				const uint8_t read_back = eeprom_read_byte(address + i);
				if (read_back != data) {
					Serial.print(F("Verification failed at 0x"));
					Serial.print(address + i, HEX);
					Serial.print(F(": wrote 0x"));
					Serial.print(data, HEX);
					Serial.print(F(", read 0x"));
					Serial.println(read_back, HEX);
					return false;
				}
			}
			break;

		case 0x01: // End of file record
			Serial.println(F("\nHex input complete."));
			return 2;

		default:
			Serial.print(F("Unsupported record type: "));
			Serial.println(record_type, HEX);
			return false;
	}

	return true;
}

/**
 * @brief Process a single character from the Intel HEX input
 * @param c Character to process
 * @return True if the character was processed successfully, false otherwise
 */
uint8_t hex_process_char(const char c) {
	// Ignore carriage return
	if (c == '\r') return true;

	if (c == '\n') {
		input_buffer[input_index] = 0; // Null terminate

		if (input_index == 0) {
			return 2;
		}

		const uint8_t status = hex_process_line(input_buffer);
		if (!status) {
			Serial.println(F("Error processing hex line!"));
		}

		input_index = 0;
		return status;
	}

	if (input_index < MAX_LINE_LENGTH) {
		input_buffer[input_index++] = c;
	}

	return true;
}

/**
 * @brief Reset the Intel HEX input buffer
 */
void hex_process_reset() {
	input_index = 0;
	memset(input_buffer, 0, sizeof(input_buffer));
}
