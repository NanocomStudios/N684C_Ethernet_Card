#include "Network.h"
#include "DataLink.h"
#include "Transport.h"
#include "defines.h"
#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"

extern uint8_t gatewayMAC[6];
extern uint8_t deviceMAC[6];
extern uint8_t broadcastMAC[6];
extern uint8_t emptyMAC[6];

uint8_t deviceIP[4]={192,168,1,17};
uint8_t gatewayIP[4]={0};
uint8_t subnetMask[4]={255,255,255,0};

uint8_t broadcastIP[4]={255,255,255,255};

static uint16_t packetID = 0;

volatile uint8_t isARPRequestd = 0;
uint8_t cacheIP[4] ={0};
uint8_t cacheMAC[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void arpReply(uint8_t* senderMAC, uint8_t* senderIP){
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

    copyArray(deviceMAC, arp->senderMAC, MAC_SIZE);
    copyArray(deviceIP, arp->senderIP, IP_SIZE);

    copyArray(senderMAC, arp->targetMAC, MAC_SIZE);
    copyArray(senderIP, arp->targetIP, IP_SIZE);

    addEthernetHeader(buffer,senderMAC, ARP_TYPE, sizeof(ARP));
}

void arpRequest(uint8_t* targetIP){
    isARPRequestd = 1;
    void* buffer = malloc(sizeof(Ethernet) + sizeof(ARP));
    ARP* arp = buffer + sizeof(Ethernet);

    arp->hardwareType[0] = 0x00;
    arp->hardwareType[1] = 0x01;

    arp->protocol[0] = 0x08;
    arp->protocol[1] = 0x00;

    arp->macAddressLength = 0x06;
    arp->ipAddressLength = 0x04;

    arp->operation[0] = 0x00;
    arp->operation[1] = 0x01;

    copyArray(deviceMAC, arp->senderMAC, MAC_SIZE);
    copyArray(deviceIP, arp->senderIP, IP_SIZE);

    copyArray(emptyMAC, arp->targetMAC, MAC_SIZE);
    copyArray(targetIP, arp->targetIP, IP_SIZE);

    copyArray(targetIP, cacheIP, IP_SIZE);

    addEthernetHeader(buffer,broadcastMAC, ARP_TYPE, sizeof(ARP));
}

void decodeARPPacket(void* buffer){
    
    uint8_t senderMAC[6];
    uint8_t senderIP[4];
    
    ARP* arp = buffer;
    if((arp->hardwareType[1] == 0x01)){
        if((arp->protocol[0] == 0x08) && (arp->protocol[1] == 0x00)){
            if((arp->operation[0] == 0x00) && (arp->operation[1] == 0x01)){
                copyArray(arp->senderMAC, senderMAC, MAC_SIZE);
                copyArray(arp->senderIP, senderIP, IP_SIZE);
                arpReply(senderMAC, senderIP);
            }else if((arp->operation[0] == 0x00) && (arp->operation[1] == 0x02)){
                if(isARPRequestd == 1){
                    if(cmpArray(arp->senderIP, cacheIP, IP_SIZE) == 1){
                        copyArray(arp->senderMAC, cacheMAC, MAC_SIZE);
                        isARPRequestd = 0;
                    }
                }
            }
        }
    }

}

void encodeIPv4(void* buffer, uint8_t protocol, uint8_t* targetIP, uint16_t length){
    
    if(cmpArray(targetIP, broadcastIP, IP_SIZE) == 1){
        copyArray(broadcastIP, cacheIP, IP_SIZE);
        copyArray(broadcastMAC, cacheMAC, MAC_SIZE);
    }
    while(cmpArray(targetIP, cacheIP, IP_SIZE) == 0 && isARPRequestd == 0){
        arpRequest(targetIP);
        sleep_ms(100);
        //while(isARPRequestd == 1){
        //    arpRequest(targetIP);
        //    sleep_ms(100);
        //}
    }
    
    encodeIPv4Common(buffer, protocol, cacheIP, cacheMAC, length);
}

void encodeIPv4Common(void* buffer, uint8_t protocol, uint8_t* targetIP, uint8_t* targetMAC, uint16_t length){
    length += sizeof(IPv4);
    uint16_t len_tmp = length;
    uint16_t id_tmp = packetID++;

    IPv4* ipHeader = buffer + sizeof(Ethernet);
    ipHeader->version_IHL = VERSION_IHL;
    ipHeader->DSCP_ECN = 0;
    changeEndien(((uint8_t*)(&len_tmp)), 2);
    copyArray(((uint8_t*)(&len_tmp)), ipHeader->length, 2);

    changeEndien(((uint8_t*)(&id_tmp)), 2);
    copyArray(((uint8_t*)(&id_tmp)), ipHeader->id, 2);

    ipHeader->flagsNfragmentOffset[0] = 0x40;
    ipHeader->flagsNfragmentOffset[1] = 0x00;

    ipHeader->timeToLive = TIME_TO_LIVE;
    ipHeader->protocol = protocol;

    copyArray(deviceIP, ipHeader->sourceIP, IP_SIZE);
    copyArray(targetIP, ipHeader->destinationIP, IP_SIZE);

    ipHeader->headerChecksum = 0;

    ipHeader->headerChecksum = calculateChecksum(ipHeader, sizeof(IPv4));

    addEthernetHeader(buffer,targetMAC, IPv4_TYPE, length);
}

void decodeIPv4(void* buffer, uint8_t* senderMAC){

    IPv4* ipHeader = buffer;
    if(cmpArray(deviceIP, ipHeader->destinationIP, IP_SIZE)){
        switch(ipHeader->protocol){
            case ICMP_PROT:
                changeEndien(ipHeader->length, 2);
                decodeIcmp(buffer + sizeof(IPv4), ipHeader->sourceIP, senderMAC, *(uint16_t*)ipHeader->length - sizeof(IPv4));
                break;
            default:

        }
    }
}

uint8_t isSameSubnet(uint8_t* targetIP){

}