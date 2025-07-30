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

    encodeIPv4(buffer, UDP_PROT, targetIP, length);
    
}

void icmpReply(uint8_t* senderIP, uint16_t* senderMAC){

}

void decodeIcmp(void* buffer, uint8_t* senderIP, uint8_t* senderMAC, uint16_t length){
    length -= sizeof(ICMP);
    ICMP* icmp = buffer;
    switch(icmp->type){
        case ICMP_ECHO_REQEST:
        replyEchoRequest(buffer + sizeof(ICMP),senderIP, senderMAC, length);
    }
}

void replyEchoRequest(ECHO* echoReq, uint8_t* senderIP, uint8_t* senderMAC, uint16_t length){
    length -= sizeof(ECHO);
    void* buffer = malloc(sizeof(ECHO) + sizeof(ICMP) + sizeof(IPv4) + sizeof(Ethernet) + length);
    ECHO* echoReply = buffer + sizeof(ICMP) + sizeof(IPv4) + sizeof(Ethernet);
    ICMP* icmp = buffer + sizeof(IPv4) + sizeof(Ethernet);

    echoReply->id = echoReq->id;
    echoReply->seq = echoReq->seq;

    uint8_t* replyData = (uint8_t*)(((void*)echoReply) + sizeof(ECHO));
    uint8_t* reqData = (uint8_t*)(((void*)echoReq) + sizeof(ECHO));
    
    copyArray(reqData, replyData, length);

    icmp->type = ICMP_ECHO_REPLY;
    icmp->code = 0;
    icmp->checkSum = 0;

    icmp->checkSum = calculateChecksum(icmp,sizeof(ECHO) + sizeof(ICMP) + length);
    printf("length : %d\n", length);
    encodeIPv4Common(buffer, ICMP_PROT, senderIP, senderMAC, sizeof(ECHO) + sizeof(ICMP) + length);
}