//
// Created by costi on 16/03/24.
//

#include "twofish.h"

#include <algorithm>

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
    uint8_t x[4], l[4][4];

    for (int j = 0; j < 4; j++) {
        x[j] = (X >> 8 * j) % (1 << 8);
#if SHOW_H_STEPS == true
        printf("\nx[%d]: %02x", j, x[j]);
#endif
    }

    for(int i = 0;i < k; i++)
        for (int j = 0; j < 4; j++) {
            l[i][j] = (L[i] >> 8 * j) % (1 << 8);
#if SHOW_H_STEPS == true
            printf("\nl[%d][%d]: %02x", i, j, l[i][j]);
#endif
        }

    uint8_t y[5][4];

    for (int j = 0; j < 4; j++)
        y[k][j] = x[j];

    if (k == 4) {
        y[3][0] = q1(y[4][0]) ^ l[3][0];
        y[3][1] = q0(y[4][1]) ^ l[3][1];
        y[3][2] = q0(y[4][2]) ^ l[3][2];
        y[3][3] = q1(y[4][3]) ^ l[3][3];
    }

    if (k >= 3) {
        y[2][0] = q1(y[3][0]) ^ l[2][0];
        y[2][1] = q1(y[3][1]) ^ l[2][1];
        y[2][2] = q0(y[3][2]) ^ l[2][2];
        y[2][3] = q0(y[3][3]) ^ l[2][3];
    }

    y[0][0] = q1(q0(q0(y[2][0]) ^ l[1][0]) ^ l[0][0]);
    y[0][1] = q0(q0(q1(y[2][1]) ^ l[1][1]) ^ l[0][1]);
    y[0][2] = q1(q1(q0(y[2][2]) ^ l[1][2]) ^ l[0][2]);
    y[0][3] = q0(q1(q1(y[2][3]) ^ l[1][3]) ^ l[0][3]);

#if SHOW_H_STEPS == true
    for (int j = 0; j < 4; j++)
        printf("\ny[%d]: %02x", j, y[0][j]);
#endif

    uint8_t z[4] {0};

    for(int i = 0;i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            uint8_t result = galMul(MDS[i][j], y[0][j], 0b101101001);
            z[i] ^= result;
        }
#if SHOW_H_STEPS == true
        printf("\nz[%d]: %02x", i, z[i]);
#endif
    }

    uint32_t Z = 0;
    for(int i = 0;i < 4; i++)
        Z += z[i] << 8 * i;

#if SHOW_H_STEPS == true
    printf("\nZ: %08x", Z);
#endif

    return Z;
}

uint32_t Twofish::g(uint32_t X) {
    uint8_t x[4];
#if SHOW_G_STEPS == true
    printf("\nX = %08x", X);
#endif

    for (int i = 0; i < 4; i++) {
        x[i] = (X >> 8 * i) % (1 << 8);
#if SHOW_G_STEPS == true
        printf("\nx[%d]: %02x", i, x[i]);
#endif
    }

    uint8_t s[4][4];

    for(int i = 0;i < k; i++)
        for (int j = 0; j < 4; j++) {
            s[i][j] = (S[i] >> 8 * j) % (1 << 8);
#if SHOW_G_STEPS == true
            printf("\ns[%d][%d]: %02x", i, j, s[i][j]);
#endif
        }

    uint8_t y[5][4];

    for (int j = 0; j < 4; j++)
        y[k][j] = x[j];

    if (k == 4) {
        y[3][0] = q1(y[4][0]) ^ s[3][0];
        y[3][1] = q0(y[4][1]) ^ s[3][1];
        y[3][2] = q0(y[4][2]) ^ s[3][2];
        y[3][3] = q1(y[4][3]) ^ s[3][3];
    }

    if (k >= 3) {
        y[2][0] = q1(y[3][0]) ^ s[2][0];
        y[2][1] = q1(y[3][1]) ^ s[2][1];
        y[2][2] = q0(y[3][2]) ^ s[2][2];
        y[2][3] = q0(y[3][3]) ^ s[2][3];
    }

    y[0][0] = q1(q0(q0(y[2][0]) ^ s[0][0]) ^ s[1][0]);
    y[0][1] = q0(q0(q1(y[2][1]) ^ s[0][1]) ^ s[1][1]);
    y[0][2] = q1(q1(q0(y[2][2]) ^ s[0][2]) ^ s[1][2]);
    y[0][3] = q0(q1(q1(y[2][3]) ^ s[0][3]) ^ s[1][3]);

    uint8_t z[4] {0};

    for(int i = 0;i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            uint8_t result = galMul(MDS[i][j], y[0][j], 0b101101001);
            z[i] ^= result;
        }
#if SHOW_G_STEPS == true
        printf("\nz[%d]: %02x", i, z[i]);
#endif
    }

    uint32_t Z = 0;
    for(int i = 0;i < 4; i++)
        Z += z[i] << 8 * i;

#if SHOW_G_STEPS == true || SHOW_G_RESULT == true
    printf("\nZ: %08x", Z);
#endif

    return Z;
}

void Twofish::F(uint32_t R0, uint32_t R1, uint8_t r, uint32_t rF[]) {
    uint64_t T0 = g(R0);
    uint64_t T1 = (R1 << 8) | (R1 >> (32 - 8));
    T1 = g(T1);
#if SHOW_T_RESULT == true
    printf("\nT0: %08x T1: %08x", T0, T1);
#endif

    uint64_t aux = T0 + T1 + K[2 * r + 8];
    rF[0] = aux % TWO32;

    aux = T0 + 2 * T1 + K[2 * r + 9];
    rF[1] =  aux % TWO32;
}

void Twofish::splitPlaintext(const uint8_t p[]) {
    for(int i = 0;i < 4; i++) {
        P[i] = 0;
        for(int j = 0; j < 4; j++)
            P[i] += p[4 * i + j] << 8 * j;
    }
#if SHOW_SPLIT_PT == true
    printf("\n");
    for(int j = 0; j < 4; j++)
        printf("P[%d]: %08x ", j, P[j]);
#endif
}

void Twofish::inputWhitening() {
    for(int i = 0;i < 4; i++)
        R[0][i] = P[i] ^ K[i];
}

void Twofish::startRounds() {
    uint32_t rF[2];
    for(uint32_t r = 0;r < 16; r++) {
        // classic


#if TEST_VECTORS == true
        //test vectors
        if (r % 2 == 0) {
            F(R[r][0], R[r][1], r, rF);

            R[r + 1][0] = R[r][0];
            R[r + 1][1] = R[r][1];

            const uint32_t aux = R[r][2] ^ rF[0];
            R[r + 1][2] = (aux >> 1) | (aux << (32 - 1));
            R[r + 1][3] = ((R[r][3] << 1) | (R[r][3] >> (32 - 1))) ^ rF[1];
        }
        else {
            F(R[r][2], R[r][3], r, rF);

            R[r + 1][2] = R[r][2];
            R[r + 1][3] = R[r][3];

            const uint32_t aux = R[r][0] ^ rF[0];
            R[r + 1][0] = (aux >> 1) | (aux << (32 - 1));
            R[r + 1][1] = ((R[r][1] << 1) | (R[r][1] >> (32 - 1))) ^ rF[1];
        }
#else
        F(R[r][0], R[r][1], r, rF);
        const uint32_t aux = R[r][2] ^ rF[0];
        R[r + 1][0] = (aux >> 1) | (aux << (32 - 1));
        R[r + 1][1] = ((R[r][3] << 1) | (R[r][3] >> (32 - 1))) ^ rF[1];
        R[r + 1][2] = R[r][0];
        R[r + 1][3] = R[r][1];
#endif

#if SHOW_ROUNDS == true
        printf("\nR[%d]", r);
        for(int i = 0;i < 4; i++)
            printf(" %08x", R[r][i]);
#endif
    }
}

void Twofish::outputWhitening() {
    for(int i = 0;i < 4; i++)
        C[i] = R[16][(i + 2) % 4] ^ K[i + 4];
}

void Twofish::createCiphertext() {
    for(int i = 0;i < 16; i++)
        c[i] = (C[i / 4] >> 8 * (i % 4)) % (1 << 8);
}

void Twofish::encrypt(const uint8_t plaintext[], uint8_t ciphertext[]) {
    splitPlaintext(plaintext);
    keySchedule();
    inputWhitening();
    startRounds();
    outputWhitening();
    createCiphertext();

    for(int i = 0;i < 16; i++)
        ciphertext[i] = c[i];
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

    for(int i = 0; i < 20; i++) {
        uint32_t A = h(2 * i * rho, Me);
        uint32_t B = h((2 * i + 1) * rho, Mo);
        B = (B << 8) | (B >> (32 - 8));

        K[2 * i] = (A + B) % TWO32;

        K[2 * i + 1] = (A + 2 * B) % TWO32;
        K[2 * i + 1] = (K[2 * i + 1] << 9) | (K[2 * i + 1] >> (32 - 9));
#if SHOW_KEY_RESULT == true
        printf("\nK[%d]: %08x K[%d]: %08x", 2 * i, K[2 * i], 2 * i + 1, K[2 * i + 1]);
#endif
    }
#if SHOW_KEY_SCHEDULE_STEPS == true || SHOW_KEY_RESULT == true
    printf("\n");
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
