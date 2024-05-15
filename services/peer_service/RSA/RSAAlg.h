//
// Created by costi on 10/04/24.
//

#ifndef RSA_RSAALG_H
#define RSA_RSAALG_H

#include <QRandomGenerator>
#include <cstdint>
#include "base64impl.h"
#include "qdebug.h"
#include <string>
#include <bitset>
#include <sstream>
#include <QList>

using namespace std;

class RSA {
private:
    uint64_t N;
    uint64_t e;
    uint64_t phi;
    uint64_t d;

    QString base64N;
    QString base64e;
    QString base64d;

    bool coprim(uint64_t nr1, uint64_t nr2) {
        uint64_t smallest = nr1 < nr2 ? nr1 : nr2;

        for (uint64_t d = 2; d <= smallest; ++d) {
            if (nr1 % d == 0 && nr2 % d == 0)
                return false;
        }

        return true;
    }

    bool isPrime(uint64_t nr) {
        if (nr <= 1)
            return false;

        if (nr <= 3)
            return true;

        if (nr % 2 == 0 || nr % 3 == 0)
            return false;

        for (uint64_t i = 5; i * i <= nr; i += 6) {
            if (nr % i == 0 || nr % (i + 2) == 0)
                return false;
        }
        return true;
    }

public:
    RSA() {
        QList<uint64_t> primes;

        for(uint64_t i = 2000; i < 6000; i++) {
            if (isPrime(i))
                primes.append(i);
        }

        uint64_t p = primes.at(QRandomGenerator::global()->bounded(primes.count()));
        uint64_t q = primes.at(QRandomGenerator::global()->bounded(primes.count()));
        N = p * q;
        phi = (p - 1) * (q - 1);

        for (int i = N - 1; i >= 2; i--) {
            if (coprim(i, phi)) {
                e = i;
                break;
            }
        }

        d = 1;
        while (d * e % phi != 1) {
            d++;
        }

        qDebug() << QString("p: %1 q: %2 N: %3 phi: %4 e: %5 d: %6")
                        .arg(QString::number(p),
                             QString::number(q),
                             QString::number(N),
                             QString::number(phi),
                             QString::number(e),
                             QString::number(d));

        base64N = QString::number(N).toUtf8().toBase64();
        base64e = QString::number(e).toUtf8().toBase64();
        base64d = QString::number(d).toUtf8().toBase64();
    }

    uint64_t encrypt(uint64_t M, uint64_t e, uint64_t N);
    uint64_t decrypt(uint64_t C);

    QString getN();
    QString getE();
    QString getD();
};

#endif //RSA_RSAALG_H
