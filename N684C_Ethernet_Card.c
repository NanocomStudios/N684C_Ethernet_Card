#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "enc28j60.h"
#include "DataLink.h"
#include "Network.h"
#include "Transport.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

extern uint8_t gatewayMAC[6];
extern uint8_t deviceMAC[6];
extern uint8_t deviceIP[4];

extern uint8_t gatewayIP[4];
extern uint8_t subnetMask[4];

uint8_t receiveBuffer[MAX_FRAMELEN + 14];

void NetworkCard(){
    uint16_t packetLength = receivePacket(receiveBuffer, MAX_FRAMELEN + 14);
    if(packetLength > 14){
        decodeEthernetPacket(receiveBuffer);
    }
}

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
    
    uint8_t tmp[] = {0x34,0xBA,0x9A,0x4F,0x31,0x89,0x20,0x03,0x25,0x61,0x03,0x40,0x08,0x06,0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x02,0x20,0x03,0x25,0x61,0x03,0x40,192,168,1,69,0x34,0xBA,0x9A,0x4F,0x31,0x89,192,168,1,1};

    init(deviceMAC);

    multicore_launch_core1(NetworkCard);
    

    while (true) {
        
        //sendPacket(tmp,sizeof(tmp) / sizeof(tmp[0]));
        //sleep_ms(1000);
    }
}
