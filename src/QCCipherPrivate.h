//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCCIPHERPRIVATE_H
#define PQC_CRYPTO_QCCIPHERPRIVATE_H

#include "QCCipher.h"

struct QCCipher {
    QCOBJECTFIELDS
    QCKeyRef privateKey;
    QCKeyRef publicKey;
    QCArrayRef saltA;
    QCArrayRef saltB;
    QCArrayRef ivSalt;
};

extern QC_STRONG QCArrayRef QCCipherGenerateMAC(QCArrayRef message, QCArrayRef token, QCArrayRef key);

/*
 * symmetric encrypt (current AES-CBC)
 */
extern QC_STRONG QCArrayRef QCCipherSymmetricEncrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef key, QCArrayRef iv);

/*
 * symmetric decrypt (current AES-CBC)
 */
extern QC_STRONG QCArrayRef QCCipherSymmetricDecrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef key, QCArrayRef iv);

#endif //PQC_CRYPTO_QCCIPHERPRIVATE_H
