//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCCIPHER_H
#define PQC_CRYPTO_QCCIPHER_H

#include "QCArray.h"
#include "QCKey.h"
#include "QCObject.h"

typedef struct QCCipher *QCCipherRef;

extern void QCCipherEncrypt(QCKeyRef publicKey, QCArrayRef message, QCArrayRef *u, QCArrayRef *v);

extern QCArrayRef QCCipherSyndrome(const QCKeyRef privateKey, QCArrayRef c0, QCArrayRef c1);

extern QCArrayRef QCCipherDecrypt(const QCKeyRef privateKey, QCArrayRef c0, QCArrayRef c1);

#endif //PQC_CRYPTO_QCCIPHER_H
