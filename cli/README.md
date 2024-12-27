# AT28C EEPROM Programmer

A Python-based command-line tool for programming AT28C series EEPROM chips using an Arduino as the hardware interface.
This tool supports reading, writing, and erasing operations with Intel HEX file format support.

## Features

- Auto-detection of Arduino ports
- Read EEPROM contents and save as Intel HEX or binary format
- Write Intel HEX files to EEPROM
- Erase EEPROM with custom patterns
- Cross-platform support (Windows, Linux, macOS)

## Prerequisites

- Python 3.6 or higher
- Required Python packages:
  ```
  pyserial
  ```

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/tvecera/at28c-eeprom-programmer.git
   cd at28c-eeprom-programmer/cli
   ```

2. Install required packages:
   ```bash
   pip install pyserial
   ```

## Usage

### List Available Arduino Ports

```bash
python eeprom_programmer.py --list
```

### Read EEPROM Contents

Read and display EEPROM contents:

```bash
python eeprom_programmer.py --read
```

Read and save to file (Intel HEX format):

```bash
python eeprom_programmer.py --read --output dump.hex
```

Read and save to file (binary format):

```bash
python eeprom_programmer.py --read --output dump.bin --format bin
```

### Write to EEPROM

Upload Intel HEX file to EEPROM:

```bash
python eeprom_programmer.py --write firmware.hex
```

### Erase EEPROM

Clean the entire EEPROM (set all bytes to 0xFF):

```bash
python eeprom_programmer.py --clean
```

### Specify Port Manually

If you have multiple Arduino boards connected, you can specify which port to use:

```bash
python eeprom_programmer.py --port COM3 --read
```

## Command Line Options

| Option               | Description                                      |
|----------------------|--------------------------------------------------|
| `--port PORT`        | Specify serial port (e.g., COM3 or /dev/ttyUSB0) |
| `--baud BAUD`        | Set baud rate (default: 115200)                  |
| `--list`             | List available Arduino ports                     |
| `--clean`            | Erase the entire EEPROM                          |
| `--read`             | Read EEPROM contents                             |
| `--output FILE`      | Output file path for read data                   |
| `--format {hex,bin}` | Output format: hex (Intel HEX) or bin (binary)   |
| `--write FILE`       | Path to Intel HEX file to upload                 |

## Technical Details

### Serial Communication

- Default baud rate: 115200
- Timeout: 1 second for normal operations, 30 seconds for erase operations
- Auto-detection of common Arduino manufacturers (Arduino, WCH.CN, FTDI)

### Intel HEX Support

The tool supports standard Intel HEX format with the following record types:

- 00: Data Record
- 01: End of File
- 02: Extended Segment Address
- 03: Start Segment Address
- 04: Extended Linear Address
- 05: Start Linear Address

## Troubleshooting

### Common Issues

1. **Arduino Not Detected**
    - Verify USB connection
    - Check if Arduino drivers are installed
    - Try a different USB port
    - Ensure no other program is using the serial port

2. **Operation Timeout**
    - Check physical connections to the EEPROM
    - Verify EEPROM is properly seated in the socket
    - Check for proper power supply voltage

3. **Write Failures**
    - Verify EEPROM is not write-protected
    - Check if the EEPROM is properly powered
    - Ensure proper voltage levels for programming
