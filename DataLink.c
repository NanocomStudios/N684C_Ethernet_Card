#include "DataLink.h"
#include "operations.h"
#include "Network.h"

uint8_t gatewayMAC[6] = {0};
uint8_t deviceMAC[6] = {0x20,0x03,0x25,0x61,0x03,0x40};

uint8_t senderMAC[6];

void addEthernetHeader(void* buffer){
    Ethernet* eth = buffer;

}

void decodeEthernetHeader(void* buffer){
    Ethernet* eth = buffer;
    copyArray(eth->sourceMac, senderMAC, sizeof(senderMAC));

    changeEndien(eth->type, 2);

    switch(*(uint16_t*)(eth->type)){
        case 0x806:
                decodeARPRequest(buffer + sizeof(Ethernet));
            break;
        default:

    }
}