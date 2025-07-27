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