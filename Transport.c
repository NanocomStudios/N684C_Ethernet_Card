#include "Transport.h"
#include "Network.h"
#include "DataLink.h"
#include "operations.h"
#include "defines.h"
#include <stdlib.h>
#include <stdio.h>

void encodeUDP(void* input, uint16_t length, uint8_t* targetIP, uint16_t sourcePort, uint16_t destinationPort){
    void* buffer = malloc(length + sizeof(UDP) + sizeof(Ethernet) + sizeof(IPv4));

    copyArray(input, (buffer + sizeof(UDP) + sizeof(Ethernet) + sizeof(IPv4)), length);

    UDP* udp = buffer + sizeof(Ethernet) + sizeof(IPv4);
    length += sizeof(UDP);
    uint16_t len_tmp = length;
    changeEndien(&sourcePort, 2);
    changeEndien(&destinationPort, 2);
    changeEndien(&len_tmp, 2);

    udp->sourcePort = sourcePort;
    udp->destinationPort = destinationPort;
    udp->length = len_tmp;
    udp->checksum = 0;

    udp->checksum = calculateChecksum(udp, length);
    changeEndien(&udp->checksum, 2);

    printf("Hi 0\n");

    encodeIPv4(buffer, UDP_PROT, targetIP, length);
    
}

void icmpReply(uint8_t* senderIP, uint16_t){

}

void decodeIcmp(void* buffer, uint8_t* senderIP){
    
}