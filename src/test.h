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

#ifndef EPROM_TEST_H
#define EPROM_TEST_H

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
void eeprom_test();

#endif //EPROM_TEST_H
