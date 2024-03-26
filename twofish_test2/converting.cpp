//
// Created by costi on 27/03/24.
//

#include "converting.h"

void stringBlockToBytes(string block, uint8_t bytes[], uint8_t blockSize) {
    for(int i = 0; i < blockSize; i++) {
        if (i < block.length())
            bytes[i] = block[i];
        else
            bytes[i] = 0;
    }
}

string bytesBlockToString(uint8_t bytes[], uint8_t blockSize) {
    string result;

    for(int i = 0;i < blockSize; i++)
        result += static_cast<char>(bytes[i]);

    return result;
}

void displayBytes(uint8_t bytes[], uint8_t blockSize) {
    for(int i = 0;i < blockSize; i++) {
        printf("0x%02x ", bytes[i]);
        if (i != 0 && (i + 1) % 8 == 0)
            printf("\n");
    }
}
