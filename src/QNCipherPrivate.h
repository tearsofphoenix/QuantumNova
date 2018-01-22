//
// Created by Isaac on 2018/1/12.
//

#ifndef QN_QNCIPHERPRIVATE_H
#define QN_QNCIPHERPRIVATE_H

#include "QNCipher.h"
#include "QNSymmetricCipher.h"

struct QNCipher {
    QNOBJECTFIELDS
    QNSymmetricCipherRef symmetricCipher;
    QNKeyRef privateKey;
    QNKeyRef publicKey;
    QNArrayRef saltA;
    QNArrayRef saltB;
    QNArrayRef ivSalt;
};

extern QN_STRONG QNArrayRef QNCipherGenerateMAC(QNArrayRef message, QNArrayRef token, QNArrayRef key);

#endif //QN_QNCIPHERPRIVATE_H
