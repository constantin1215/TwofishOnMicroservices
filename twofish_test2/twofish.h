//
// Created by costi on 16/03/24.
//

#ifndef TWOFISH_H
#define TWOFISH_H

#define SHOW_Q_STEPS false
#define SHOW_KEY_SCHEDULE_STEPS false
#define SHOW_KEY_RESULT false
#define SHOW_H_STEPS false
#define SHOW_G_STEPS false
#define SHOW_G_RESULT false
#define SHOW_T_RESULT false
#define SHOW_ROUNDS false
#define SHOW_SPLIT_PT false
#define TEST_VECTORS false

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

constexpr unsigned long long TWO32 = 4294967296;

class Twofish {
    uint8_t key[32]{};
    uint16_t N = 128;
    uint8_t k = N / 64;
    uint32_t M[8] {};
    uint32_t Me[4] {};
    uint32_t Mo[4] {};
    uint32_t P[4] {};
    uint32_t C[4] {};
    uint32_t R[18][4] {};
    uint32_t K[40] {};
    uint32_t S[4] {};
    uint32_t rho = (1 << 24) + (1 << 16) + (1 << 8) + 1;
    uint8_t c[16];
public:
    Twofish(const uint16_t newN, const uint8_t newKey[]) {
        N = newN;
        k = N / 64;

        for (int i = 0;i < N / 8; i++)
            key[i] = newKey[i];

        // keySchedule();
    }
    void setKey(uint8_t newKey[]) {
        for (int i = 0;i < N / 8; i++)
            key[i] = newKey[i];
    }
    void showKey() const {
        printf("KEY=");
        for (int i = 0;i < N / 8; i++)
            printf("%02x", key[i]);
        printf("\n");
        //printf("\nN: %d\n", N);
        //printf("k: %d\n", k);
    }
    void showCiphertext() const {
        printf("CT=");
        for (int i = 0;i < 16; i++)
            printf("%02x", c[i]);
        printf("\n");
    }
private:
    uint8_t q(uint8_t byte,const uint8_t t0[],const uint8_t t1[],const uint8_t t2[],const uint8_t t3[]);
    uint8_t q0(uint8_t byte);
    uint8_t q1(uint8_t byte);
    uint32_t h(uint32_t X, uint32_t L[]);
    uint32_t g(uint32_t X);
    void F(uint32_t R0, uint32_t R1, uint8_t r, uint32_t F[]);
    void splitPlaintext(const uint8_t plaintext[]);
    void inputWhitening();
    void startRounds();
    void outputWhitening();
    void createCiphertext();
    void keySchedule();
public:
    void encrypt(const uint8_t plaintext[], uint8_t ciphertext[]);
};

#endif //TWOFISH_H
