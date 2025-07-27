#ifndef ENC28J60
#define ENC28J60

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define ADDRESS_OFFSET_MASK 0x1F    //00011111
#define ADDRESS_BANK_MASK 0x60      //01100000

#define READ_CONTROL_REG_OP 0x00
#define READ_MEMORY_BUFFER_OP 0x3A
#define WRITE_CONTROL_REG_OP 0x40
#define WRITE_MEMORY_BUFFER_OP 0x7A
#define BIT_FIELD_SET_OP 0x80
#define BIT_FIELD_CLEAR_OP 0xA0
#define RESET_OP 0xFF

#define ECON1 0x1F

void reset();
void setCs(uint8_t inp);
void spiWriteByte(uint8_t inp);
void writeOnCurrentBank(uint8_t opCode, uint8_t address, uint8_t data);
uint8_t readOnCurrentBank(uint8_t opCode, uint8_t address);
void readBuffer(uint8_t* buffer, uint16_t length);
void writeBuffer(uint8_t* buffer, uint16_t length);
void setMemoryBank(uint8_t address);
uint8_t readByte(uint8_t address);
void writeByte(uint8_t address, uint8_t data);

#endif