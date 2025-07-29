#include "Network.h"
#include "DataLink.h"
#include "operations.h"
#include <stdlib.h>

extern uint8_t gatewayMAC[6];
extern uint8_t deviceMAC[6];

uint8_t deviceIP[4]={0};

uint8_t senderMAC[6];
uint8_t senderIP[4];

void arpReply(){
    void* buffer = malloc(sizeof(Ethernet) + sizeof(ARP));
    ARP* arp = buffer + sizeof(Ethernet);

    arp->hardwareType[0] = 0x00;
    arp->hardwareType[1] = 0x01;

    arp->protocol[0] = 0x08;
    arp->protocol[1] = 0x00;

    arp->macAddressLength = 0x06;
    arp->ipAddressLength = 0x04;

    arp->operation[0] = 0x00;
    arp->operation[1] = 0x02;

    copyArray(deviceMAC, arp->senderMAC, sizeof(deviceMAC));
    copyArray(deviceIP, arp->senderIP, sizeof(deviceIP));

    copyArray(senderMAC, arp->targetMAC, sizeof(senderMAC));
    copyArray(senderIP, arp->targetIP, sizeof(senderIP));
}

void decodeARPRequest(void* buffer){
    ARP* arp = buffer;
}