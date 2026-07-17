# PicoTemp — A Producer/Consumer Temperature Sensor Walkthrough

This guide walks through a simple embedded project that reads a **MAX6675 thermocouple sensor** on a **Raspberry Pi Pico (RP2040)**, sends raw data over USB, and decodes it on a **PC or laptop**.

---

## High-Level Model

```
┌─────────────────────────┐        USB serial         ┌──────────────────────┐
│  Pico (producer)        │ ────────────────────────> │  PC / laptop         │
│                         │   "1234,5678\n"           │  (consumer / reader) │
│  picotemp.c             │                           │  reader.c            │
│    └─ max6675.c/.h      │                           │                      │
│                         │                           │  Parses "ms,counts"  │
│  Reads MAX6675 sensor   │                           │  Converts to °C / °F │
│  via SPI every ~1 sec   │                           │  Prints to terminal  │
└─────────────────────────┘                           └──────────────────────┘
```

**The problem:** Measure a high-temperature thermocouple (e.g. 0–1023.75 °C) using a MAX6675 chip, and print the temperature on a PC screen.

**The split:**
- The **Pico** handles the low-level hardware (SPI, GPIO, timing) and sends **raw counts** over USB.
- The **PC** handles the human-friendly conversion and display. It doesn't need special hardware — just a serial port.

This keeps the embedded firmware simple and the math (conversion) in a place where it's easy to modify, test, and re-run without re-flashing the Pico.

---

## File-by-File Tour

### `max6675.h` — The Driver Interface (header)

```c
typedef struct {
    spi_inst_t *spi;
    uint cs_pin;
} max6675_t;

void max6675_init(max6675_t *dev, spi_inst_t *spi, uint cs_pin);
uint16_t max6675_read_raw(max6675_t *dev);
float max6675_read_temp_c(max6675_t *dev);
```

Key idea: **We encapsulate the sensor as a C struct.** The struct holds the two pieces of hardware knowledge the driver needs:

| Field     | Purpose                                  |
|-----------|------------------------------------------|
| `spi`     | Which SPI peripheral (spi0 or spi1)      |
| `cs_pin`  | Which GPIO pin drives Chip Select        |

This is a lightweight version of **object-oriented programming in C** — the struct is "the object," and the functions that take a pointer to it are "the methods."

The header declares three public functions:
- `init` — set up the hardware
- `read_raw` — get the 16-bit register value from the MAX6675
- `read_temp_c` — read and convert to Celsius

### `max6675.c` — The Driver Implementation

```c
void max6675_init(max6675_t *dev, spi_inst_t *spi, uint cs_pin)
{
    dev->spi = spi;           // remember which SPI bus
    dev->cs_pin = cs_pin;     // remember which CS pin

    gpio_init(cs_pin);
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1);      // keep CS high (inactive)
}
```

`init` stores the configuration in the struct and sets up the CS pin. The struct now "knows" how to talk to *this specific* sensor instance.

```c
uint16_t max6675_read_raw(max6675_t *dev)
{
    uint8_t rx[2];

    gpio_put(dev->cs_pin, 0);   // pull CS low → select sensor
    spi_read_blocking(dev->spi, 0, rx, 2);  // read 2 bytes
    gpio_put(dev->cs_pin, 1);   // pull CS high → deselect

    return ((uint16_t)rx[0] << 8) | rx[1];  // combine into 16-bit
}
```

This is the **SPI transaction** — the messy part of talking to silicon. It:
1. Pulls the Chip Select line low (the sensor is listening)
2. Shifts two dummy bytes while clocking in the sensor's 16-bit reply
3. Pulls CS high (transaction done)
4. Packs the two bytes into a `uint16_t`

```c
float max6675_read_temp_c(max6675_t *dev)
{
    uint16_t raw = max6675_read_raw(dev);
    uint16_t counts = (raw >> 3) & 0x0FFF;  // bits 3–14 are the reading
    return counts * 0.25f;                  // each count = 0.25 °C
}
```

`read_temp_c` builds on `read_raw`. It extracts the 12-bit temperature field and multiplies by the scale factor from the datasheet (0.25 °C per count).

**Why separate `read_raw` and `read_temp_c`?**
- The Pico **producer** may only want raw counts (simpler, no floating-point on the Pico).
- The PC **reader** can do the float math without burdening the embedded device.
- It gives the caller choice: do you want raw data or processed data?

### `picotemp.c` — The Producer (Pico firmware)

```c
spi_init(spi0, 500000);  // 500 kHz SPI clock
spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

max6675_t temp_sensor;
max6675_init(&temp_sensor, spi0, PICO_DEFAULT_SPI_CSN_PIN);
```

This is where the driver is **instantiated and wired up**. We declare a `max6675_t` variable and pass it the SPI bus and CS pin. From this point on, the rest of the code never touches SPI or GPIO directly — it all goes through the driver.

```c
while (true) {
    uint16_t raw = max6675_read_raw(&temp_sensor);
    uint16_t counts = (raw >> 3) & 0x0FFF;

    uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;

    printf("%u,%u\n", elapsed_ms, counts);
    sleep_ms(1000);
}
```

The main loop:
1. Reads the raw 16-bit value from the sensor
2. Extracts the 12-bit temperature counts
3. Prints a CSV line: `milliseconds,counts`
4. Waits 1 second

The output goes over USB serial (configured in `CMakeLists.txt` via `pico_enable_stdio_usb`). Every second the Pico spits out a line like:

```
4387,2830
5387,2841
6387,2856
```

### `reader.c` — The Consumer (PC program)

```c
int fd = open("/dev/cu.usbmodem11401", O_RDONLY);
```

Opens the USB serial device. The exact path depends on your OS:
- **macOS:** `/dev/cu.usbmodemXXXXXX`
- **Linux:** `/dev/ttyACM0` or `/dev/ttyUSB0`
- **Windows:** a COM port (you'd use a different API or WSL)

```c
while (1) {
    if (read_line(fd, line, sizeof(line)) > 0) {
        sscanf(line, "%u,%u", &ms, &counts);
        float temp_c = counts * 0.25f;
        float temp_f = temp_c * 9.0f / 5.0f + 32.0f;
        printf("C=%f F=%f\n", temp_c, temp_f);
    }
}
```

The reader:
1. Reads one line at a time (its own `read_line` helper, because the standard library doesn't have one for file descriptors)
2. Parses the two comma-separated values with `sscanf`
3. Converts counts to Celsius (`counts * 0.25`) and then to Fahrenheit
4. Prints the result

**No special hardware knowledge needed.** The reader doesn't know about SPI, CS pins, or thermocouples — it just does arithmetic on numbers.

---

## How It All Comes Together

### Build the Pico firmware

```bash
mkdir build && cd build
cmake ..
make
```

Flash `picotemp.uf2` to the Pico (hold BOOTSEL, plug in, drag the file).

### Build and run the reader

```bash
gcc -o reader reader.c
./reader
```

You'll see:
```
C=707.500000 F=1305.500000
C=710.250000 F=1310.449951
C=714.000000 F=1317.199951
```

---

## Summary of the Architecture

| Concern               | Where            | Why there                           |
|-----------------------|------------------|-------------------------------------|
| SPI protocol details  | `max6675.c`      | Only the driver needs this          |
| Sensor initialization | `max6675.c`      | Part of the driver                  |
| Sensor state          | `max6675_t` struct | The driver's "object"              |
| Timing + main loop    | `picotemp.c`     | The producer's orchestration        |
| Raw → counts logic    | both sides       | `picotemp.c` does it, `reader.c` re-does it for learning clarity |
| Serial output         | `picotemp.c`     | `printf` via USB stdio              |
| Serial input + parse  | `reader.c`       | The consumer's job                  |
| °C / °F conversion    | `reader.c`       | Keeps floating-point off the Pico   |

The struct-as-driver pattern is the linchpin: it lets you separate **what the sensor does** from **how it talks to the hardware**. You can swap the MAX6675 for a different SPI sensor by writing a new driver with the same kind of struct and init/read functions — the main loop doesn't need to change.
