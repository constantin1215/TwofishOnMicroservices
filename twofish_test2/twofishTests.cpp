//
// Created by costi on 17/03/24.
//

#include "twofishTests.h"

#include "keys.h"
#include "twofish.h"

void basicTest128() {
    Twofish twofish(ZerosKey.N, ZerosKey.key);
    uint8_t ciphertext[16];
    twofish.encrypt(testPlaintext, ciphertext);

    for(int i = 0;i < 16; i++)
        if (ciphertext[i] != expectedCiphertextForTest128[i]) {
            printf("\nbasicTest128 FAILED!\n");
            return;
        }

    printf("\nbasicTest128 PASSED!\n");
}

void basicTest192() {
    Twofish twofish(TestKey1.N, TestKey1.key);
    uint8_t ciphertext[16];
    twofish.encrypt(testPlaintext, ciphertext);

    // twofish.showKey();
    twofish.showCiphertext();

    for(int i = 0;i < 16; i++)
        if (ciphertext[i] != expectedCiphertextForTest192[i]) {
            printf("\nbasicTest192 FAILED!\n");
            return;
        }

    printf("\nbasicTest192 PASSED!\n");
}

void basicTest256() {
    Twofish twofish(TestKey2.N, TestKey2.key);
    uint8_t ciphertext[16];
    twofish.encrypt(testPlaintext, ciphertext);

    for(int i = 0;i < 16; i++)
        if (ciphertext[i] != expectedCiphertextForTest256[i]) {
            printf("\nbasicTest256 FAILED!\n");
            return;
        }

    printf("\nbasicTest256 PASSED!\n");
}

void runFullEncryptions128() {
    Twofish twofish(ZerosKey.N, ZerosKey.key);
    uint8_t ciphertext[16];
    twofish.encrypt(testPlaintext, ciphertext);

    uint8_t plaintext[16];
    uint8_t key[16];

    for(int i = 0;i < 16; i++)
        plaintext[i] = testPlaintext[i];

    printf("IT=1\n");
    twofish.showKey();

    printf("PT=");
    for(int i = 0;i < 16; i++)
        printf("%02x", plaintext[i]);
    printf("\n");

    twofish.showCiphertext();

    for(int i = 1; i < 49; i++) {
        for (int i = 0; i < 16; i++) {
            key[i] = plaintext[i];
            plaintext[i] = ciphertext[i];
        }

        twofish.setKey(key);
        twofish.encrypt(plaintext, ciphertext);

        printf("IT=%d\n", i + 1);
        twofish.showKey();

        printf("PT=");
        for(int i = 0;i < 16; i++)
            printf("%02x", plaintext[i]);
        printf("\n");

        twofish.showCiphertext();
    }
}

void encryptDecrypt1MB() {
    uint8_t blocks[1][16] = {
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        }
    };
    uint8_t blocksNr = 90000;

    Twofish twofish(ZerosKey.N, ZerosKey.key);
    uint8_t result[16];
    uint8_t cipherBlocks[100000][16];

    printf("\n CYPHER \n");

    twofish.encrypt(IV, result);

    // for(int i = 0;i < ZerosKey.N / 8; i++) {
    //     printf("%02x", result[i]);
    // }
    // printf("\n");

    for (int j = 0; j < blocksNr; j++) {
        for(int i = 0;i < ZerosKey.N / 8; i++) {
            cipherBlocks[j][i] = result[i] ^ blocks[0][i];
            printf("%02x", cipherBlocks[j][i]);
        }
        printf("\n");
        twofish.encrypt(cipherBlocks[j], result);
    }

    printf("\n DECYPHER \n");

    twofish.encrypt(IV, result);

    // for(int i = 0;i < ZerosKey.N / 8; i++) {
    //     printf("%02x", result[i] ^ cipherBlocks[0][i]);
    // }
    // printf("\n");

    uint8_t decypheredBlock[100000][16];

    for (int j = 0; j < blocksNr; j++) {
        for(int i = 0;i < ZerosKey.N / 8; i++) {
            decypheredBlock[j][i] = result[i] ^ cipherBlocks[j][i];
            printf("%02x", decypheredBlock[j][i]);
        }
        printf("\n");
        twofish.encrypt(cipherBlocks[j], result);
    }
}
