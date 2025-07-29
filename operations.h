#ifndef OPERATIONS
#define OPERATIONS

typedef unsigned char uint8_t;

void copyArray(void* arrayCopy, void* arrayPaste, int length);
uint8_t cmpArray(void* array1, void* array2, int length);
void changeEndien(void* array, int length);

#endif