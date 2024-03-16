//
// Created by costi on 16/03/24.
//

#include "twofish.h"
#include "galois.h"

uint8_t Twofish::q(const uint8_t byte, const uint8_t t0[], const uint8_t t1[], const uint8_t t2[],
          const uint8_t t3[]) {
    const uint8_t a0 = byte / 16;
    const uint8_t b0 = byte % 16;

    const uint8_t a1 = a0 ^ b0;
    const bitset<4> aux((b0 >> 1 | b0 << 3));
    const uint8_t b1 = a0 ^ aux.to_ulong() ^ 8 * a0 % 16;

    const uint8_t a2 = t0[a1];
    const uint8_t b2 = t1[b1];

    const uint8_t a3 = a2 ^ b2;
    const bitset<4> aux2((b2 >> 1 | b2 << 3));
    const uint8_t b3 = a2 ^ aux2.to_ulong() ^ 8 * a2 % 16;

    const uint8_t a4 = t2[a3];
    const uint8_t b4 = t3[b3];

#if SHOW_Q_STEPS == true
    cout << "a0: " << static_cast<int>(a0) << " b0: " << static_cast<int>(b0) << endl;
    cout << "a1: " << static_cast<int>(a1) << " b1: " << static_cast<int>(b1) << endl;
    cout << "a2: " << static_cast<int>(a2) << " b2: " << static_cast<int>(b2) << endl;
    cout << "a3: " << static_cast<int>(a3) << " b3: " << static_cast<int>(b3) << endl;
    cout << "a4: " << static_cast<int>(a4) << " b4: " << static_cast<int>(b4) << endl;
#endif

    return 16 * b4 + a4;
}

uint8_t Twofish::q0(const uint8_t byte) {
    const uint8_t t0[] = { 0x8, 0x1, 0x7, 0xD, 0x6, 0xF, 0x3, 0x2, 0x0, 0xB, 0x5, 0x9, 0xE, 0xC, 0xA, 0x4 };
    const uint8_t t1[] = { 0xE, 0xC, 0xB, 0x8, 0x1, 0x2, 0x3, 0x5, 0xF, 0x4, 0xA, 0x6, 0x7, 0x0, 0x9, 0xD };
    const uint8_t t2[] = { 0xB, 0xA, 0x5, 0xE, 0x6, 0xD, 0x9, 0x0, 0xC, 0x8, 0xF, 0x3, 0x2, 0x4, 0x7, 0x1 };
    const uint8_t t3[] = { 0xD, 0x7, 0xF, 0x4, 0x1, 0x2, 0x6, 0xE, 0x9, 0xB, 0x3, 0x0, 0x8, 0x5, 0xC, 0xA };
    return q(byte, t0, t1, t2, t3);
}

uint8_t Twofish::q1(const uint8_t byte) {
    const uint8_t t0[] = { 0x2, 0x8, 0xB, 0xD, 0xF, 0x7, 0x6, 0xE, 0x3, 0x1, 0x9, 0x4, 0x0, 0xA, 0xC, 0x5 };
    const uint8_t t1[] = { 0x1, 0xE, 0x2, 0xB, 0x4, 0xC, 0x3, 0x7, 0x6, 0xD, 0xA, 0x5, 0xF, 0x9, 0x0, 0x8 };
    const uint8_t t2[] = { 0x4, 0xC, 0x7, 0x5, 0x1, 0x6, 0x9, 0xA, 0x0, 0xE, 0xD, 0x8, 0x2, 0xB, 0x3, 0xF };
    const uint8_t t3[] = { 0xB, 0x9, 0x5, 0x1, 0xC, 0x3, 0xD, 0xE, 0x6, 0x4, 0x7, 0xF, 0x2, 0x0, 0x8, 0xA };
    return q(byte, t0, t1, t2, t3);
}

uint32_t Twofish::h(uint32_t X, uint32_t L[]) {
    return 0;
}

void Twofish::keySchedule() {
    for(int i = 0; i < 2 * k; i++) {
        uint32_t aux = 0;
        for (int j = 0; j < 4; j++) {
            aux += key[4 * i + j] << 8 * j;
#if SHOW_KEY_SCHEDULE_STEPS == true
            printf("Step %d: M[%d]: %08x\n", j, i, aux);
#endif
        }
        M[i] = aux;
    }

    int i1 = 0, i2 = 0;
    for (int i = 0;i < 2 * k; i++)
        if (i % 2 == 0)
            Me[i1++] = M[i];
        else
            Mo[i2++] = M[i];

    uint32_t s[4][4] { 0 };
    for(int i = 0;i < k; i++) {
        for(int j = 0;j < 4; j++)
            for(int l = 0;l < 8; l++) {
                uint8_t result = galMul(RS[j][l], key[8 * i + l], 0b101001101);
                s[i][j] ^= result;
#if SHOW_KEY_SCHEDULE_STEPS == true
                printf("s[%d][%d]: %08x \n", i, j, s[i][j]);
#endif
            }
    }

    for (int i = 0;i < k; i++) {
        S[i] = 0;
        for(int j = 0;j < 4; j++) {
            S[i] += s[i][j] << 8 * j;
#if SHOW_KEY_SCHEDULE_STEPS == true
            printf("Step %d: S[%d]: %08x\n", j, i, S[i]);
#endif
        }
    }

#if SHOW_KEY_SCHEDULE_STEPS == true || SHOW_KEY_RESULT == true
    for (int i = 0;i < k; i++)
        printf("Me[%d]: %08x ", i, Me[i]);
    printf("\n");
    for (int i = 0;i < k; i++)
        printf("Mo[%d]: %08x ", i, Mo[i]);
    printf("\n");
    for (int i = 0;i < k; i++)
        printf("S[%d]: %08x ", i, S[i]);
#endif
}
