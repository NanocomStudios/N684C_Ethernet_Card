#ifndef DATA_LINK
#define DATA_LINK

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct Ethernet{
    uint8_t destinationMac[6];
    uint8_t sourceMac[6];
    uint8_t type[2];
}Ethernet;

void decodeEthernetPacket(void* buffer);
void addEthernetHeader(void* buffer, uint8_t destinationMAC[], uint16_t type, uint16_t length);

#endif