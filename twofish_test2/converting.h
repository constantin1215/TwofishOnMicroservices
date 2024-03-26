//
// Created by costi on 27/03/24.
//

#ifndef CONVERTING_H
#define CONVERTING_H

#include <string>

using namespace std;

void stringBlockToBytes(string block, uint8_t bytes[], uint8_t blockSize);
string bytesBlockToString(uint8_t bytes[], uint8_t blockSize);
void displayBytes(uint8_t bytes[], uint8_t blockSize);

#endif //CONVERTING_H
