#ifndef TRANSPORT
#define TRANSPORT

typedef unsigned char uint8_t;

typedef struct UDP{
    uint8_t sourcePort[2];
    uint8_t destinationPort[2];
    uint8_t length[2];
    uint8_t checksum[2];
}UDP;

#endif