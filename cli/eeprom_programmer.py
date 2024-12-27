# MIT License
#
# Copyright (c) 2024 Tomas Vecera, tomas@vecera.dev
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#  *
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#  *
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import serial
import serial.tools.list_ports
import time
from typing import List, Tuple, Dict
import sys


def find_arduino_ports() -> List[Dict[str, str]]:
    """Find all Arduino devices connected to the system

    Returns:
        List of dictionaries containing port information:
        - port: Port name (e.g., 'COM3' or '/dev/ttyUSB0')
        - desc: Device description
        - hwid: Hardware ID
        - manufacturer: Manufacturer name (if available)
    """
    arduino_ports = []

    for serial_port in serial.tools.list_ports.comports():
        # Common Arduino manufacturers/identifiers
        if any(id in serial_port.manufacturer.lower() if serial_port.manufacturer else False
               for id in ['arduino', 'wch.cn', 'ftdi']):
            arduino_ports.append({
                'port': serial_port.device,
                'desc': serial_port.description,
                'hwid': serial_port.hwid,
                'manufacturer': serial_port.manufacturer or 'Unknown'
            })

    return arduino_ports


def calculate_checksum(line: List[int]) -> int:
    """Calculate Intel HEX checksum for a list of bytes

    Args:
        line: List of integers (0-255) to checksum

    Returns:
        Checksum byte (0-255)
    """
    # Sum all bytes
    total = sum(line) & 0xFF
    # Two's complement
    return ((~total) + 1) & 0xFF


def bytes_to_hex_record(address: int, record_type: int, line: List[int]) -> str:
    """Convert bytes to Intel HEX record string

    Intel HEX record format:
    :LLAAAATT[DD...]CC
    Where:
    : - Start of record
    LL - Length of data (1-2 digits)
    AAAA - Address (4 digits)
    TT - Record type (2 digits)
        00 - Data record
        01 - End of file
        02 - Extended segment address
        03 - Start segment address
        04 - Extended linear address
        05 - Start linear address
    DD - Data bytes
    CC - Checksum

    Args:
        address: Starting address for this record (0-65535)
        record_type: Record type (0-255)
        line: List of bytes (0-255) for this record

    Returns:
        Intel HEX record string including checksum

    Raises:
        ValueError: If address > 0xFFFF or any data byte > 0xFF
    """
    if address > 0xFFFF:
        raise ValueError(f"Address {address:X} exceeds 0xFFFF")
    if any(b > 0xFF for b in line):
        raise ValueError("Data contains bytes > 0xFF")
    if record_type > 0xFF:
        raise ValueError(f"Record type {record_type:X} exceeds 0xFF")

    # Prepare record fields
    byte_count = len(line)
    addr_hi = (address >> 8) & 0xFF
    addr_lo = address & 0xFF

    # Build list of all bytes that contribute to checksum
    checksum_data = [
                        byte_count,  # Length
                        addr_hi,  # Address high byte
                        addr_lo,  # Address low byte
                        record_type  # Record type
                    ] + line  # Data bytes

    # Calculate checksum
    checksum = calculate_checksum(checksum_data)

    # Format record string
    hex_data = ''.join(f'{b:02X}' for b in line)
    return f":{byte_count:02X}{addr_hi:02X}{addr_lo:02X}{record_type:02X}{hex_data}{checksum:02X}"


class EEPROMProgrammerError(Exception):
    """Custom exception for EEPROM programmer errors"""
    pass


class ArduinoClient:
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1.0):
        """Initialize EEPROM programmer client

        Args:
            port: Serial port name (e.g., 'COM3' or '/dev/ttyUSB0')
            baudrate: Baud rate (default: 115200)
            timeout: Serial timeout in seconds (default: 1.0)
        """
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2)  # Wait for Arduino reset
        self._clear_buffer()

    def _clear_buffer(self):
        """Clear the serial input buffer"""
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def _send_command(self, cmd: str) -> str:
        """Send a command and return the response

        Args:
            cmd: Single character command

        Returns:
            Response string from the programmer
        """
        self._clear_buffer()
        self.ser.write(cmd.encode())
        time.sleep(0.1)

        response = []
        timeout = time.time() + 5  # 5 second timeout

        while True:
            if self.ser.in_waiting:
                line = self.ser.readline().decode().strip()
                if line:  # Only append non-empty lines
                    response.append(line)
                    break

            # Check for timeout
            if time.time() > timeout:
                raise EEPROMProgrammerError("Timeout waiting for Arduino response")

            time.sleep(0.1)

        return '\n'.join(response)

    def _send_hex_value(self, value: int, digits: int = 4) -> None:
        """Send a hex value when prompted

        Args:
            value: Integer value to send
            digits: Number of hex digits (default: 4)
        """
        hex_str = f"{value:0{digits}X}"
        self.ser.write(hex_str.encode())
        self.ser.write(b'\r\n')
        time.sleep(0.1)

    def erase(self, start_addr: int = 0, end_addr: int = 0, pattern: int = 0xFF):
        """Erase EEPROM section with specified pattern

        Args:
            start_addr: Start address (default: 0)
            end_addr: End address (default: 0 = full size)
            pattern: Byte pattern to write (default: 0xFF)
        """
        # Send initial command
        self._send_command('E')

        # Send parameters
        self._send_hex_value(start_addr)
        self._send_hex_value(end_addr)
        self._send_hex_value(pattern, digits=2)

        # Wait for completion response
        timeout = time.time() + 30  # 30 second timeout for erase

        while True:
            if self.ser.in_waiting:
                line = self.ser.readline().decode().strip()
                if line.endswith("Commands:"):
                    break
                if line:
                    print(line)

            # Check for timeout
            if time.time() > timeout:
                raise EEPROMProgrammerError("Timeout waiting for erase completion")

            time.sleep(0.1)

    def read(self, start_addr: int = 0) -> List[Tuple[int, List[int]]]:
        """Dump EEPROM contents

        Args:
            start_addr: Start address for dump (default: 0)

        Returns:
            List of tuples containing (address, [bytes])
        """
        self._send_command('D')
        self._send_hex_value(start_addr)

        dump_data = []
        current_addr = None
        current_bytes = []

        while True:
            if self.ser.in_waiting:
                line = self.ser.readline().decode().strip()

                if line.endswith('>'):
                    if current_addr is not None and current_bytes:
                        dump_data.append((current_addr, current_bytes))
                    break

                if not line or line.startswith('Addr'):
                    continue

                if line.startswith('Press'):  # Continue dump after page break
                    self.ser.write(b' ')
                    time.sleep(0.1)
                elif ': ' in line:
                    print(line)
                    if current_addr is not None and current_bytes:
                        dump_data.append((current_addr, current_bytes))
                    addr_str, data_str = line.split(': ')
                    current_addr = int(addr_str, 16)
                    current_bytes = []

                    for byte_str in data_str.split():
                        if byte_str:
                            current_bytes.append(int(byte_str, 16))

        return dump_data

    def write_hex(self, hex_data: str):
        """Write Intel HEX format data

        Args:
            hex_data: Intel HEX format string (can be multiple lines)

        Returns:
            Status message
        """
        self._send_command('W')
        time.sleep(0.2)

        for line in hex_data.strip().split('\n'):
            self.ser.write(line.strip().encode() + b'\r\n')
            time.sleep(0.1)
            while True:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode().strip()
                    if line.startswith('Commands'):
                        break
                    print(line)
                else:
                    break

        # Send empty line to finish
        self.ser.write(b'\r\n')

    def close(self):
        """Close serial connection"""
        self.ser.close()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='AT28C EEPROM Programmer CLI')
    parser.add_argument('--port', help='Serial port (if not specified, will try to detect)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate')
    parser.add_argument('--list', action='store_true', help='List available Arduino ports')
    parser.add_argument('--clean', action='store_true', help='Clean (erase) the entire EEPROM')
    parser.add_argument('--read', action='store_true', help='Read EEPROM contents')
    parser.add_argument('--output', type=str, help='Output file path for read data')
    parser.add_argument('--format', choices=['hex', 'bin'], default='hex',
                        help='Output format: hex (Intel HEX) or bin (binary)')
    parser.add_argument('--write', type=str, help='Path to Intel HEX file to upload')
    args = parser.parse_args()

    if args.list:
        print("\nDetected Arduino ports:")
        ports = find_arduino_ports()
        if not ports:
            print("No Arduino devices found")
        else:
            for i, port_info in enumerate(ports, 1):
                print(f"\n{i}. Port: {port_info['port']}")
                print(f"   Description: {port_info['desc']}")
                print(f"   Manufacturer: {port_info['manufacturer']}")
        sys.exit(0)

    arduino_port = args.port
    if not arduino_port:
        # Try to automatically find an Arduino port
        ports = find_arduino_ports()
        if not ports:
            print("Error: No Arduino devices found!")
            sys.exit(1)
        if len(ports) > 1:
            print("\nMultiple Arduino devices found. Please specify one:")
            for i, port_info in enumerate(ports, 1):
                print(f"{i}. {port_info['port']} - {port_info['desc']}")
            try:
                choice = int(input("\nSelect device (1-{}): ".format(len(ports))))
                arduino_port = ports[choice - 1]['port']
            except (ValueError, IndexError):
                print("Invalid selection!")
                sys.exit(1)
        else:
            arduino_port = ports[0]['port']
            print(f"\nUsing detected Arduino port: {arduino_port}")

    programmer = None
    try:
        try:
            programmer = ArduinoClient(arduino_port, args.baud)
        except serial.SerialException as e:
            print(f"Error connecting to port {arduino_port}: {str(e)}")
            sys.exit(1)
        except Exception as e:
            print(f"Unexpected error initializing programmer: {str(e)}")
            sys.exit(1)

        # Perform operations
        if args.clean:
            print("\nCleaning EEPROM...")
            try:
                # Send erase command and display raw response
                programmer.erase()
            except Exception as e:
                print(f"Error cleaning EEPROM: {str(e)}")
                sys.exit(1)

        if args.read:
            print("\nRead EEPROM data...")
            try:
                data = programmer.read()
                # Save to file if output path specified
                if args.output:
                    try:
                        if args.format == 'bin':
                            # Save as binary file
                            with open(args.output, 'wb') as f:
                                for _, bytes_data in data:
                                    f.write(bytes(bytes_data))
                            print(f"\nSaved binary data to {args.output}")

                        else:  # hex format
                            # Save as Intel HEX file
                            with open(args.output, 'w') as f:
                                for addr, bytes_data in data:
                                    # Write data in 16-byte chunks
                                    for chunk_start in range(0, len(bytes_data), 16):
                                        chunk = bytes_data[chunk_start:chunk_start + 16]
                                        record = bytes_to_hex_record(addr + chunk_start, 0, chunk)
                                        f.write(record + '\n')

                                # Write end-of-file record
                                f.write(':00000001FF\n')
                            print(f"\nSaved Intel HEX data to {args.output}")

                    except Exception as e:
                        print(f"Error saving file: {str(e)}")
                        sys.exit(1)

            except Exception as e:
                print(f"Error reading EEPROM: {str(e)}")
                sys.exit(1)

        if args.write:
            try:
                with open(args.write, 'r') as f:
                    result = f.read()

                print(f"\nUploading hex file: {args.write}")
                programmer.write_hex(result)
                print("Upload complete")
            except FileNotFoundError:
                print(f"Error: Hex file not found: {args.write}")
                sys.exit(1)
            except Exception as e:
                print(f"Error uploading hex file: {str(e)}")
                sys.exit(1)

    finally:
        if programmer is not None:
            try:
                programmer.close()
            except Exception as e:
                print(f"Error closing programmer: {str(e)}")
