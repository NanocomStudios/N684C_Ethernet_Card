#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "enc28j60.h"

static uint8_t currentMemoryBank = 0;
static uint16_t NextPacketPtr;

void setCs(uint8_t inp){
    gpio_put(PIN_CS,!inp);
}

void reset(){
    setCs(1);
    spiWriteByte(RESET_OP);
    setCs(0);
}

void spiWriteByte(uint8_t inp){
    spi_write_blocking(SPI_PORT, &inp, 1);
}

void writeOnCurrentBank(uint8_t opCode, uint8_t address, uint8_t data){
    setCs(1);

    spiWriteByte(opCode | (address & ADDRESS_OFFSET_MASK));
    spiWriteByte(data);

    setCs(0);
}

uint8_t readOnCurrentBank(uint8_t opCode, uint8_t address){
    setCs(1);

    spiWriteByte(opCode | (address & ADDRESS_OFFSET_MASK));
    uint8_t dst[1];
	spi_read_blocking(SPI_PORT, 0, dst, 1);

    if(currentMemoryBank >= 2){
        spi_read_blocking(SPI_PORT, 0, dst, 1);
    }

    setCs(0);
    return (dst[0]);
}

void readBuffer(uint8_t* buffer, uint16_t length){
    setCs(1);

    spiWriteByte(READ_MEMORY_BUFFER_OP);
    spi_read_blocking(SPI_PORT, 0, buffer, length);

    setCs(0);
}

void writeBuffer(uint8_t* buffer, uint16_t length){
    setCs(1);

    spiWriteByte(WRITE_MEMORY_BUFFER_OP);
    spi_write_blocking(SPI_PORT, buffer, length);

    setCs(0);
}

void setMemoryBank(uint8_t address){
    if(((address & ADDRESS_BANK_MASK) >> 5) != currentMemoryBank){
        currentMemoryBank = ((address & ADDRESS_BANK_MASK) >> 5);
        writeOnCurrentBank(BIT_FIELD_CLEAR_OP, ECON1, 3);   //0000 0011
        writeOnCurrentBank(BIT_FIELD_SET_OP, ECON1, currentMemoryBank);
    }
}

uint8_t readByte(uint8_t address){
    setMemoryBank(address);
    return readOnCurrentBank(READ_CONTROL_REG_OP, address);
}

void writeByte(uint8_t address, uint8_t data){
    setMemoryBank(address);
    writeOnCurrentBank(WRITE_CONTROL_REG_OP, address, data);
}

void writePHY(uint8_t address, uint16_t data){
	// set the PHY register address
	writeByte(MIREGADR, address);
	// write the PHY data
	writeByte(MIWRL, data);
	writeByte(MIWRH, data >> 8);
	// wait until the PHY write completes
	while (readByte(MISTAT) & MISTAT_BUSY)
	{
		sleep_ms(15);
	}
}

void sendPacket(uint8_t* packet, uint16_t length){
    // Set the write pointer to start of transmit buffer area
    writeByte(EWRPTL, TXSTART_INIT & 0xFF);
    writeByte(EWRPTH, TXSTART_INIT >> 8);

    // Set the TXND pointer to correspond to the packet size given
    writeByte(ETXNDL, (TXSTART_INIT + length) & 0xFF);
	writeByte(ETXNDH, (TXSTART_INIT + length) >> 8);

    // write per-packet control byte (0x00 means use macon3 settings)
	writeOnCurrentBank(WRITE_MEMORY_BUFFER_OP, 0, 0x00);
	// copy the packet into the transmit buffer
	writeBuffer(packet, length);

	// send the contents of the transmit buffer onto the network
	writeOnCurrentBank(BIT_FIELD_SET_OP, ECON1, ECON1_TXRTS);
	// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
	// http://ww1.microchip.com/downloads/en/DeviceDoc/80349c.pdf
	if ((readByte(EIR) & EIR_TXERIF))
	{
		writeOnCurrentBank(BIT_FIELD_CLEAR_OP, ECON1, ECON1_TXRTS);
	}
}

uint16_t receivePacket(uint8_t* buffer, uint16_t maxLength){
    uint16_t rxstat;
	uint16_t len;

    // check if a packet has been received and buffered
    if (readByte(EPKTCNT) == 0)
	{
		return (0);
	}

    // Set the read pointer to the start of the received packet
	writeByte(ERDPTL, (NextPacketPtr));
	writeByte(ERDPTH, (NextPacketPtr) >> 8);
	// read the next packet pointer
	NextPacketPtr = readOnCurrentBank(READ_MEMORY_BUFFER_OP, 0);
	NextPacketPtr |= readOnCurrentBank(READ_MEMORY_BUFFER_OP, 0) << 8;
	// read the packet length (see datasheet page 43)
	len = readOnCurrentBank(READ_MEMORY_BUFFER_OP, 0);
	len |= readOnCurrentBank(READ_MEMORY_BUFFER_OP, 0) << 8;
	len -= 4; //remove the CRC count
	// read the receive status (see datasheet page 43)
	rxstat = readOnCurrentBank(READ_MEMORY_BUFFER_OP, 0);
	rxstat |= readOnCurrentBank(READ_MEMORY_BUFFER_OP, 0) << 8;
	// limit retrieve length
	if (len > maxLength - 1)
	{
		len = maxLength - 1;
	}
	// check CRC and symbol errors (see datasheet page 44, table 7-3):
	// The ERXFCON.CRCEN is set by default. Normally we should not
	// need to check this.
	if ((rxstat & 0x80) == 0)
	{
		// invalid
		len = 0;
	}
	else
	{
		// copy the packet from the receive buffer
		readBuffer(buffer, len);
	}
	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	writeByte(ERXRDPTL, (NextPacketPtr));
	writeByte(ERXRDPTH, (NextPacketPtr) >> 8);
	// decrement the packet counter indicate we are done with this packet
	writeOnCurrentBank(BIT_FIELD_SET_OP, ECON2, ECON2_PKTDEC);
	return (len);
}

void init(uint8_t* macaddr){

    setCs(0);

    reset();

    sleep_ms(100);

    // set receive buffer start address
    NextPacketPtr = RXSTART_INIT;

    // Rx start
	writeByte(ERXSTL, RXSTART_INIT & 0xFF);
	writeByte(ERXSTH, RXSTART_INIT >> 8);
	// set receive pointer address
	writeByte(ERXRDPTL, RXSTART_INIT & 0xFF);
	writeByte(ERXRDPTH, RXSTART_INIT >> 8);
	// RX end
	writeByte(ERXNDL, RXSTOP_INIT & 0xFF);
	writeByte(ERXNDH, RXSTOP_INIT >> 8);
	// TX start
	writeByte(ETXSTL, TXSTART_INIT & 0xFF);
	writeByte(ETXSTH, TXSTART_INIT >> 8);
	// TX end
	writeByte(ETXNDL, TXSTOP_INIT & 0xFF);
	writeByte(ETXNDH, TXSTOP_INIT >> 8);
	// do bank 1 stuff, packet filter:
	// For broadcast packets we allow only ARP packtets
	// All other packets should be unicast only for our mac (MAADR)
	//
	// The pattern to match on is therefore
	// Type     ETH.DST
	// ARP      BROADCAST
	// 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
	// in binary these poitions are:11 0000 0011 1111
	// This is hex 303F->EPMM0=0x3f,EPMM1=0x30
	writeByte(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
	writeByte(EPMM0, 0x3f);
	writeByte(EPMM1, 0x30);
	writeByte(EPMCSL, 0xf9);
	writeByte(EPMCSH, 0xf7);
	//
	//
	// do bank 2 stuff
	// enable MAC receive
	writeByte(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	// bring MAC out of reset
	writeByte(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	writeOnCurrentBank(BIT_FIELD_SET_OP, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
	// set inter-frame gap (non-back-to-back)
	writeByte(MAIPGL, 0x12);
	writeByte(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	writeByte(MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
	// Do not send packets longer than MAX_FRAMELEN:
	writeByte(MAMXFLL, MAX_FRAMELEN & 0xFF);
	writeByte(MAMXFLH, MAX_FRAMELEN >> 8);
	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	writeByte(MAADR5, macaddr[0]);
	writeByte(MAADR4, macaddr[1]);
	writeByte(MAADR3, macaddr[2]);
	writeByte(MAADR2, macaddr[3]);
	writeByte(MAADR1, macaddr[4]);
	writeByte(MAADR0, macaddr[5]);
	// no loopback of transmitted frames
	writePHY(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	setMemoryBank(ECON1);
	// enable interrutps
	writeOnCurrentBank(BIT_FIELD_SET_OP, EIE, EIE_INTIE | EIE_PKTIE);
	// enable packet reception
	writeOnCurrentBank(BIT_FIELD_SET_OP, ECON1, ECON1_RXEN);
}