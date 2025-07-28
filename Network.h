#ifndef NETWORK
#define NETWORK

typedef unsigned char uint8_t;

typedef struct ARP{
    uint8_t hardwareType[2];
    uint8_t protocol[2];
    uint8_t macAddressLength[1];
    uint8_t ipAddressLength[1];
    uint8_t operation[2];
    uint8_t senderMAC[6];
    uint8_t senderIP[4];
    uint8_t targetMAC[6];
    uint8_t targetIP[4];
}ARP;

typedef struct IPv4{
    uint8_t version_IHL[1];
    uint8_t DSCP_ECN[1];
    uint8_t length[2];
}IPv4;

#endif