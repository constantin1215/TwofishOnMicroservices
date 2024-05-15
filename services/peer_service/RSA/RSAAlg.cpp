//
// Created by costi on 10/04/24.
//

#include "RSAAlg.h"

uint64_t pow_div_conq(uint64_t nr, uint64_t power, uint64_t modulo) {
    uint64_t extra = 1;
    while(power != 1) {
        if (power % 2 == 1) {
            extra *= nr;
        }
        nr = nr * nr % modulo;
        power = power / 2;
        extra %= modulo;
    }

    return (nr * extra) % modulo;
}

uint64_t RSA::encrypt(uint64_t M, uint64_t e, uint64_t N) {
    return pow_div_conq(M, e, N);
}

uint64_t RSA::decrypt(uint64_t C) {
    return pow_div_conq(C, d, N);
}

QString RSA::getN()
{
    return base64N;
}

QString RSA::getE()
{
    return base64e;
}

QString RSA::getD()
{
    return base64d;
}
