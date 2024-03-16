//
// Created by costi on 16/03/24.
//

#include "galois.h"

#include <algorithm>
#include <bitset>
#include <cstdio>
#include <iostream>

using namespace std;

uint8_t msb(uint16_t x) {
    uint8_t poz = 0;
    for(uint8_t i = 0; i < 16; i++) {
        if((1 << i) & x)
            poz = i;
    }
    return poz;
}

uint16_t galMul(uint8_t x, uint8_t y, uint16_t mod) {
    uint16_t xx = x;
    uint16_t yy = y;
    uint16_t result = 0;
    for(uint8_t i = 0; i < 8; i++) {
        if((1 << i) & yy) {
            result = result ^ (xx << i);
        }
    }
    // find position of msb 1-bit for result and for mod
    uint8_t poz_mod = msb(mod), poz_result = msb(result);
    //cout << (int)poz_mod << ' ' << (int)poz_result << "\n";
    if(poz_result >= poz_mod) {
        while(poz_result >= 8) {
            uint8_t i = msb(result) - msb(mod);

            //cout << bitset<16>(result) << "\n";
            //cout << bitset<16>(mod << i) << "\n\n";

            result = result ^ (mod << i);

            poz_result = msb(result);
        }
    }

    return result;
}