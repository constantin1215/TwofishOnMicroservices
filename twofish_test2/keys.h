//
// Created by costi on 16/03/24.
//

#ifndef KEYS_H
#define KEYS_H
#include <cstdint>

constexpr uint8_t testPlaintext[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

constexpr uint8_t IV[16] = {
    0xCF, 0xD1, 0xD2, 0xE5, 0xA9, 0xBE, 0x9C, 0xDF,
    0x50, 0x1F, 0x13, 0xB8, 0x92, 0xBD, 0x22, 0x48
};

constexpr uint8_t expectedCiphertextForTest128[16] {
    0x9F, 0x58, 0x9F, 0x5C, 0xF6, 0x12, 0x2C, 0x32,
    0xB6, 0xBF, 0xEC, 0x2F, 0x2A, 0xE8, 0xC3, 0x5A
};

constexpr uint8_t expectedCiphertextForTest192[16] {
    0xCF, 0xD1, 0xD2, 0xE5, 0xA9, 0xBE, 0x9C, 0xDF,
    0x50, 0x1F, 0x13, 0xB8, 0x92, 0xBD, 0x22, 0x48
};

constexpr uint8_t expectedCiphertextForTest256[16] {
    0x37, 0x52, 0x7B, 0xE0, 0x05, 0x23, 0x34, 0xB8,
    0x9F, 0x0C, 0xFC, 0xCA, 0xE8, 0x7C, 0xFA, 0x20
};

constexpr struct {
    uint8_t key[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t N = 128;
} ZerosKey;

constexpr struct {
    uint8_t key[24] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
    };
    uint8_t N = 192;
} TestKey1;

constexpr struct {
    uint8_t key[32] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    uint16_t N = 256;
} TestKey2;

#endif //KEYS_H
