#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
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

extern mutex_t spiLock;

uint8_t receiveBuffer[MAX_FRAMELEN + 14];

void NetworkCard(){
    while(1){
        uint16_t packetLength = receivePacket(receiveBuffer, MAX_FRAMELEN + 14);
        if(packetLength > 14){
            decodeEthernetPacket(receiveBuffer);
        }
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
    
    uint8_t tmp[] = "Hello World";

    uint8_t senderMAC[6] = {0x00,0xe0,0x4c,0x68,0x02,0x13};
    uint8_t senderIP[4] = {192,168,1,5};

    mutex_init(&spiLock);

    init(deviceMAC);

    multicore_launch_core1(NetworkCard);
    
    while (true) {
        encodeUDP(tmp, sizeof(tmp), senderIP, 80,80);
        //arpRequest(senderIP);
        //arpReply(senderMAC,senderIP);
        //sendPacket(tmp,sizeof(tmp) / sizeof(tmp[0]));
        sleep_ms(1000);
    }
}
