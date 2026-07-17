#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "max6675.h"

int main() {
    stdio_init_all();

    spi_init(spi0, 500000);

    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); 

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN,  GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);

    max6675_t temp_sensor;
    max6675_init(&temp_sensor, spi0, PICO_DEFAULT_SPI_CSN_PIN);


    uint32_t start_ms = to_ms_since_boot(get_absolute_time());

    while (true) {
      
      uint16_t raw = max6675_read_raw(&temp_sensor);
      uint16_t counts = (raw >> 3) & 0x0FFF;

      uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_ms;

      printf("%u,%u\n", elapsed_ms, counts);

      sleep_ms(1000);
    }
}
