#include "DataLink.h"
#include "operations.h"

uint8_t gatewayMAC[6] = {0};
uint8_t deviceMAC[6] = {0x20,0x03,0x25,0x61,0x03,0x40};

void addEthernetHeader(void* buffer){
    Ethernet* eth = buffer;

}

void decodeEthernetHeader(void* buffer){
    Ethernet* eth = buffer;
    
}