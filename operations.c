#include "operations.h"

void copyArray(void* arrayCopy, void* arrayPaste, int length){
    uint8_t* arrC = arrayCopy;
    uint8_t* arrP = arrayPaste;

    for(int i = 0; i < length; i++){
        arrP[i] = arrC[i];
    }

}

uint8_t cmpArray(void* array1, void* array2, int length){
    uint8_t* arr1 = array1;
    uint8_t* arr2 = array2;

    for(int i = 0; i < length; i++){
        if(arr1[i] != arr2[i]){
            return 0;
        }
    }
    return 1;
}

void changeEndien(void* array, int length){
    uint8_t* arr = array;
    uint8_t tmp;
    length--;
    for(int i = 0; i < length; i++){
        tmp = arr[i];
        arr[i] = arr[length];
        arr[length] = tmp;
        length--;
    }
}