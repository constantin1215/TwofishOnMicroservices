//
// Created by costi on 16/03/24.
//

#ifndef TWOFISH_H
#define TWOFISH_H

#define SHOW_Q_STEPS true
#define SHOW_KEY_SCHEDULE_STEPS true
#define SHOW_KEY_RESULT true

#include <iostream>
#include <bitset>

using namespace std;

constexpr uint8_t RS[4][8] = {
    { 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E },
    { 0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5 },
    { 0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19 },
    { 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03 }
};

constexpr uint8_t MDS[4][4] = {
    { 0x01, 0xEF, 0x5B, 0x5B },
    { 0x5B, 0xEF, 0xEF, 0x01 },
    { 0xEF, 0x5B, 0x01, 0xEF },
    { 0xEF, 0x01, 0xEF, 0x5B }
};

class Twofish {
    uint8_t key[32]{};
    uint16_t N = 128;
    uint8_t k = N / 64;
    uint32_t M[8] {};
    uint32_t Me[4] {};
    uint32_t Mo[4] {};
    uint32_t S[4] {};
public:
    Twofish(const uint16_t newN, const uint8_t newKey[]) {
        N = newN;
        k = N / 64;

        for (int i = 0;i < N / 8; i++)
            key[i] = newKey[i];
    }
    void showKey() const {
        for (int i = 0;i < N / 8; i++)
            printf("Key[%d]: 0x%02x\n", i, key[i]);
        printf("N: %d\n", N);
    }
    void keySchedule();
private:
    uint8_t q(uint8_t byte,const uint8_t t0[],const uint8_t t1[],const uint8_t t2[],const uint8_t t3[]);
    uint8_t q0(uint8_t byte);
    uint8_t q1(uint8_t byte);
    uint32_t h(uint32_t X, uint32_t L[]);
};

#endif //TWOFISH_H
