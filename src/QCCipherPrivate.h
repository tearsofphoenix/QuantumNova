//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCCIPHERPRIVATE_H
#define PQC_CRYPTO_QCCIPHERPRIVATE_H

#include "QCCipher.h"
#include "QNSymmetricCipher.h"

struct QCCipher {
    QCOBJECTFIELDS
    QNSymmetricCipherRef symmetricCipher;
    QCKeyRef privateKey;
    QCKeyRef publicKey;
    QCArrayRef saltA;
    QCArrayRef saltB;
    QCArrayRef ivSalt;
};

extern QC_STRONG QCArrayRef QCCipherGenerateMAC(QCArrayRef message, QCArrayRef token, QCArrayRef key);

#endif //PQC_CRYPTO_QCCIPHERPRIVATE_H
