#include "converting.h"
#include "keys.h"
#include "twofish.h"
#include "twofishTests.h"
#include <fstream>
#include <sstream>

ifstream fin("lorem.txt");
ofstream fout("result.txt");

int main() {
    //basicTest128();
    // basicTest192();
    // basicTest256();

    //runFullEncryptions128(); // vezi doc

    // string plaintext = "ana are mere si pere";
    string plaintext;
    string buffer;

    if (fin.is_open()) {
        while (getline(fin, buffer)) {
            plaintext += buffer + '\n';
        }
        fin.close();
    }
    else {
        printf("Error opening file.");
        return -1;
    }

    plaintext = "hi";
    uint32_t nrOfBytes = plaintext.length();
    const uint8_t BLOCK_SIZE = 16;
    printf("Plaintext: %s\n", plaintext.c_str());
    printf("The string has: %d bytes\n", nrOfBytes);
    uint8_t nrOfBlocks = nrOfBytes % BLOCK_SIZE == 0 ? nrOfBytes / BLOCK_SIZE : nrOfBytes / BLOCK_SIZE + 1;
    printf("We have to encrypt: %d blocks\n\n", nrOfBlocks);

    uint8_t bytes[10000][16] = { 0 };
    uint8_t ciphertext[10000][16];
    uint8_t intermediateResult[16];

    Twofish twofish(ZerosKey.N, ZerosKey.key);
    twofish.encrypt(IV, intermediateResult);

    for(int i = 0; i < nrOfBlocks; i++) {
        string block = plaintext.substr(i * 16, 16);
        printf("BLOCK %d:\nPlaintext: %s\n", i + 1, block.c_str());

        stringBlockToBytes(block, bytes[i], BLOCK_SIZE);
        printf("Bytes:\n\n");
        displayBytes(bytes[i], BLOCK_SIZE);
        printf("\n");

        printf("Ciphertext:\n\n");
        for (int j = 0; j < BLOCK_SIZE; j++)
            ciphertext[i][j] = intermediateResult[j] ^ bytes[i][j];
        displayBytes(ciphertext[i], BLOCK_SIZE);
        printf("\n");
        twofish.encrypt(ciphertext[i], intermediateResult);
        // printf("Ciphertext:\n\n");
        // printf("%s\n", bytesBlockToString(cyphertext[i], BLOCK_SIZE).c_str());
    }

    printf("DECYPHERING\n\n");
    twofish.encrypt(IV, intermediateResult);

    uint8_t decypheredBlocks[10000][16];

    string finalResult;
    for(int i = 0; i < nrOfBlocks; i++) {
        printf("BLOCK %d:\n", i + 1);
        printf("Bytes:\n\n");
        displayBytes(ciphertext[i], BLOCK_SIZE);
        printf("\n");

        for (int j = 0; j < BLOCK_SIZE; j++)
            decypheredBlocks[i][j] = intermediateResult[j] ^ ciphertext[i][j];
        twofish.encrypt(ciphertext[i], intermediateResult);
        printf("Decyphered: \n\n");
        displayBytes(decypheredBlocks[i], BLOCK_SIZE);
        printf("\n");
        string decipheredString = bytesBlockToString(decypheredBlocks[i], BLOCK_SIZE);
        printf("%s", decipheredString.c_str());
        finalResult += decipheredString;
        printf("\n");;

    }

    printf("Final result:\n%s", finalResult.c_str());
    finalResult.resize(finalResult.find('\0'));
    fout << finalResult;

    return 0;
}
