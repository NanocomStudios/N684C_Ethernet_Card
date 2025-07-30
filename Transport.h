#ifndef TRANSPORT
#define TRANSPORT

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct UDP{
    uint16_t sourcePort;
    uint16_t destinationPort;
    uint16_t length;
    uint16_t checksum;
}UDP;

void encodeUDP(void* input, uint16_t length, uint8_t* targetIP, uint16_t sourcePort, uint16_t destinationPort);

#endif