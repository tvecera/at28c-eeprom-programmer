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

#ifndef EEPROM_AT28C_H
#define EEPROM_AT28C_H

#define EEPROM_SIZE  8192  // 8KB for AT28C64
#define ADDR_BITS    13    // A0-A12
#define CHIP_NAME    "AT28C64"

#if CHIP_TYPE == 256
#define EEPROM_SIZE  32768 // 32KB for AT28C256
#define ADDR_BITS    15    // A0-A14
#define CHIP_NAME    "AT28C256"
#endif

/**
 * @brief Initialize the EEPROM programmer. This function must be called before any other EEPROM functions.
 * @return True if initialization was successful, false otherwise
 */
bool eeprom_init();

/**
 * @brief Write a byte to the EEPROM and use data polling to verify the write operation.
 * @param address EEPROM address
 * @param data Data to write
 */
void eeprom_write_byte(uint16_t address, uint8_t data);

/**
 * @brief Read a byte from the EEPROM.
 * @param address EEPROM address
 * @return Data read from the EEPROM
 */
uint8_t eeprom_read_byte(uint16_t address);

/**
 * @brief Verify a byte in the EEPROM against an expected value.
 * @param address EEPROM address
 * @param expected Expected data
 * @return True if the data matches, false otherwise
 */
bool eeprom_verify_byte(uint16_t address, uint8_t expected);

/**
 * @brief Enable or disable EEPROM write protection. This function is not available on all AT28C EEPROMs.
 * @param enable Enable write protection
 */
void eeprom_write_protect(bool enable);


/**
 * @brief Erase a section of the EEPROM by writing a pattern to all bytes.
 * @param start Start address
 * @param end End address
 * @param pattern Pattern to write
 */
void eeprom_erase_section(uint16_t start, uint16_t end, uint8_t pattern);

#endif //EEPROM_AT28C_H
