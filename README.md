# PicoTemp 🌡️

**Embedded temperature sensing on the Raspberry Pi Pico — with a clean producer/consumer architecture.**

PicoTemp reads a [MAX6675 thermocouple sensor](https://www.analog.com/media/en/technical-documentation/data-sheets/max6675.pdf) over SPI and streams raw temperature data over USB serial to a PC for real-time display.

```
┌──────────────────────┐    USB Serial     ┌──────────────────────┐
│  Pico (producer)     │ ────────────────> │  PC (consumer)       │
│  picotemp.c          │   "ms,counts\n"   │  reader.c            │
│    └─ max6675.c/.h   │                   │                      │
│                      │                   │  counts → °C / °F    │
│  SPI → MAX6675       │                   │  print to terminal   │
└──────────────────────┘                   └──────────────────────┘
```

## Features

- **Producer/Consumer decoupling** — the Pico handles hardware, the PC handles math
- **Raw SPI data** streamed as CSV over USB serial (no floating-point on the Pico)
- **Real-time temperature conversion** on the PC side (Celsius & Fahrenheit)
- **Reusable sensor driver** — the `max6675_t` struct pattern makes it easy to swap sensors
- **Zero external dependencies** on the reader side — pure POSIX C

## Project Structure

```
├── picotemp.c          # Pico firmware — main loop, SPI init, data output
├── max6675.c           # MAX6675 sensor driver — SPI transactions
├── max6675.h           # Driver header — struct + public API
├── reader.c            # PC reader — serial input, parsing, conversion
├── CMakeLists.txt      # CMake build config for Pico firmware
└── GUIDE.md            # Detailed walkthrough of the code
```

## How It Works

### The Producer (Pico)

The Pico firmware initializes the SPI bus, configures the MAX6675 sensor, and loops forever:

1. Reads the raw 16-bit register from the MAX6675 via SPI
2. Extracts the 12-bit temperature counts
3. Prints `elapsed_ms,counts` over USB serial
4. Waits 1 second

### The Consumer (PC)

The PC reader opens the USB serial device and loops:

1. Reads a line of CSV data
2. Parses `ms` and `counts`
3. Converts counts to °C (`counts × 0.25`) and °F
4. Prints the result to the terminal

## Getting Started

### Prerequisites

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) (for the firmware)
- A Raspberry Pi Pico (RP2040)
- A MAX6675 thermocouple module
- A C compiler (for the reader — any POSIX system)

### Build the Firmware

```bash
mkdir build && cd build
cmake ..
make
```

Flash `picotemp.uf2` to the Pico (hold BOOTSEL, plug in, drag the file).

### Build & Run the Reader

```bash
gcc -o reader reader.c
./reader
```

Set the serial device path in `reader.c` to match your system:
| OS      | Typical Path                   |
|---------|--------------------------------|
| macOS   | `/dev/cu.usbmodemXXXXXX`       |
| Linux   | `/dev/ttyACM0` / `/dev/ttyUSB0`|
| Windows | COM port (WSL or Cygwin)       |

### See the Output

```
C=707.500000 F=1305.500000
C=710.250000 F=1310.449951
C=714.000000 F=1317.199951
```

## Architecture

| Concern                 | Component        | Why                        |
|-------------------------|------------------|----------------------------|
| SPI protocol            | `max6675.c`      | Only the driver needs it   |
| Sensor state            | `max6675_t`      | Driver's "object"          |
| Timing & main loop      | `picotemp.c`     | Producer orchestration     |
| Raw data → CSV          | `picotemp.c`     | USB serial output          |
| CSV parsing & display   | `reader.c`       | Consumer responsibility    |
| °C / °F conversion      | `reader.c`       | Keeps float off the Pico   |

## The Driver Pattern

The MAX6675 driver uses a lightweight **object-oriented C** pattern — a struct holds hardware configuration, and functions that take a pointer to it act as methods:

```c
typedef struct {
    spi_inst_t *spi;
    uint cs_pin;
} max6675_t;

void     max6675_init(max6675_t *dev, spi_inst_t *spi, uint cs_pin);
uint16_t max6675_read_raw(max6675_t *dev);
float    max6675_read_temp_c(max6675_t *dev);
```

This keeps the hardware details encapsulated and the main loop clean.

## License

See [LICENSE](LICENSE).
