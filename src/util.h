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

#ifndef EEPROM_UTIL_H
#define EEPROM_UTIL_H

inline void print_execution_time(unsigned long elapsed_time) __attribute__((always_inline));

inline void print_progress(uint16_t addr) __attribute__((always_inline));

inline uint8_t hex_char_to_int(char c) __attribute__((always_inline));

inline uint32_t get_hex_value(uint8_t size, uint8_t empty) __attribute__((always_inline));

/**
 * @brief Print execution time in minutes and seconds
 * @param elapsed_time Time in milliseconds
 */
void print_execution_time(const unsigned long elapsed_time) {
	unsigned long seconds = elapsed_time / 1000;
	const unsigned long minutes = seconds / 60;
	seconds = seconds % 60;

	Serial.print(F("Execution time: "));
	Serial.print(minutes);
	Serial.print(F(" minutes, "));
	Serial.print(seconds);
	Serial.println(F(" seconds"));
}

/**
 * @brief Print progress indicator
 * @param addr Current address
 */
void print_progress(const uint16_t addr) {
	if ((addr & 0x0F) == 0x0F) {
		Serial.print(F("."));
	}
	// Check if we've hit 1024 bytes
	if ((addr & 0x3FF) == 0x3FF) {
		Serial.println();
	}
}

/**
 * @brief Convert hex character to integer
 * @param c Hex character
 * @return Integer value
 */
uint8_t hex_char_to_int(const char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}

uint32_t get_hex_value(const uint8_t size, const uint8_t empty) {
	delay(10);
	while (Serial.available()) Serial.read();
	char c = 0;
	char buff[size + 1] = {};

	uint8_t pos = 0;
	while (true) {
		if (Serial.available()) {
			c = static_cast<char>(Serial.read());

			// On Enter, process the value
			if (c == '\n' || c == '\r') {
				delay(10);
				while (Serial.available()) Serial.read();
				if (pos == 0) return empty; // Empty input
				break;
			}

			// Only accept hex digits
			if (((c >= '0' && c <= '9') ||
			     (c >= 'A' && c <= 'F') ||
			     (c >= 'a' && c <= 'f'))) {
				if (pos == size) {
					for (uint8_t i = 0; i < size - 1; i++) {
						buff[i] = buff[i + 1];
					}
					buff[size - 1] = static_cast<char>(toupper(c));
				} else {
					buff[pos++] = static_cast<char>(toupper(c));
				}
				Serial.print(c); // Echo the character
			}
		}
	}
	auto input = String(buff);
	// If empty input, return 0
	if (input.length() == 0) {
		return 0;
	}

	// Take last x characters (or all if less than x)
	const int start_pos = max(0, (int)input.length() - size);
	input = input.substring(start_pos);

	// Convert hex string to integer
	uint32_t address = 0;
	for (unsigned int i = 0; i < input.length(); i++) {
		c = static_cast<char>(toupper(input.charAt(i)));
		address <<= 4;
		if (c >= '0' && c <= '9') {
			address |= (c - '0');
		} else if (c >= 'A' && c <= 'F') {
			address |= (c - 'A' + 10);
		}
	}

	// Clear input buffer
	delay(10);
	while (Serial.available()) Serial.read();
	return address;
}

#endif //EEPROM_UTIL_H
