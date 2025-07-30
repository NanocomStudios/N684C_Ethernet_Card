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

typedef struct ICMP{
    uint8_t type;
    uint8_t code;
    uint16_t checkSum;
}ICMP;

typedef struct ECHO{
    uint16_t id;
    uint16_t seq;
}ECHO;

void encodeUDP(void* input, uint16_t length, uint8_t* targetIP, uint16_t sourcePort, uint16_t destinationPort);
void decodeIcmp(void* buffer, uint8_t* senderIP, uint8_t* senderMAC, uint16_t length);
void replyEchoRequest(ECHO* echoReq, uint8_t* senderIP, uint8_t* senderMAC, uint16_t length);

#endif