#include "DataLink.h"
#include "operations.h"
#include "Network.h"
#include "defines.h"
#include "enc28j60.h"
#include <stdlib.h>

uint8_t gatewayMAC[6] = {0};
uint8_t deviceMAC[6] = {0x20,0x03,0x25,0x61,0x03,0x40};

uint8_t broadcastMAC[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t emptyMAC[6] = {0};

void addEthernetHeader(void* buffer, uint8_t destinationMAC[], uint16_t type, uint16_t length){
    Ethernet* eth = buffer;
    copyArray(destinationMAC, eth->destinationMac, MAC_SIZE);
    copyArray(deviceMAC, eth->sourceMac, MAC_SIZE);
    
    changeEndien((uint8_t*)(&type), 2);
    *((uint16_t*)(eth->type)) = type;

    sendPacket(buffer, length + sizeof(Ethernet));
    free(buffer);
}

void decodeEthernetPacket(void* buffer){
    Ethernet* eth = buffer;

    changeEndien(eth->type, 2);

    switch(*(uint16_t*)(eth->type)){
        case 0x806:
                decodeARPPacket(buffer + sizeof(Ethernet));
            break;
        default:

    }
}