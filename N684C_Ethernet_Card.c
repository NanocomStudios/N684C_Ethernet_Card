#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "enc28j60.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments


int main()
{
    stdio_init_all();

    spi_init(SPI_PORT, 2000000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    
    uint8_t mac[] = {0x20,0x03,0x25,0x61,0x03,0x40};
    uint8_t tmp[] = {0xff,0xff,0xff,0xff,0xff,0xff,0x20,0x03,0x25,0x61,0x03,0x40,0x00,0x2,0xaa,0x55};

    init(mac);

    
    while (true) {
        sendPacket(tmp,sizeof(tmp) / sizeof(tmp[0]));
        sleep_ms(1000);
    }
}
