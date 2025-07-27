#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "enc28j60.h"

uint8_t currentMemoryBank = 0;

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

