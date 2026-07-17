#ifndef MAX6675_H
#define MAX6675_H

#include <stdint.h>
#include "hardware/spi.h"

typedef struct {
    spi_inst_t *spi;
    uint cs_pin;
} max6675_t;

void max6675_init(max6675_t *dev,
                  spi_inst_t *spi,
                  uint cs_pin);

uint16_t max6675_read_raw(max6675_t *dev);

float max6675_read_temp_c(max6675_t *dev);

#endif
