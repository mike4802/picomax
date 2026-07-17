#include "max6675.h"

#include "hardware/gpio.h"

void max6675_init(max6675_t *dev,
                  spi_inst_t *spi,
                  uint cs_pin)
{
    dev->spi = spi;
    dev->cs_pin = cs_pin;

    gpio_init(cs_pin);
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1);
}

uint16_t max6675_read_raw(max6675_t *dev)
{
    uint8_t rx[2];

    gpio_put(dev->cs_pin, 0);

    spi_read_blocking(dev->spi, 0, rx, 2);

    gpio_put(dev->cs_pin, 1);

    return ((uint16_t)rx[0] << 8) | rx[1];
}

float max6675_read_temp_c(max6675_t *dev)
{
    uint16_t raw = max6675_read_raw(dev);

    uint16_t counts = (raw >> 3) & 0x0FFF;

    return counts * 0.25f;
}
