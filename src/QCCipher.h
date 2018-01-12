//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCCIPHER_H
#define PQC_CRYPTO_QCCIPHER_H

#include "QCArray.h"
#include "QCKey.h"
#include "QCObject.h"

typedef struct QCCipher *QCCipherRef;

extern QCCipherRef QCCipherCreate(void);

extern QCKeyRef QCCipherGetPrivateKey(QCCipherRef cipher);

extern QCKeyRef QCCipherGetPublicKey(QCCipherRef cipher);

extern void QCCipherSetPrivateKey(QCCipherRef cipher, QCCipherRef privateKey);

extern void QCCipherSetPublicKey(QCCipherRef cipher, QCCipherRef publicKey);

extern void QCCipherEncrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef *u, QCArrayRef *v);

extern QCArrayRef QCCipherSyndrome(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1);

extern QCArrayRef QCCipherDecrypt(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1);

#endif //PQC_CRYPTO_QCCIPHER_H
