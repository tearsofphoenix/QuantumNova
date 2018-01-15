//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCCIPHER_H
#define PQC_CRYPTO_QCCIPHER_H

#include "QCArray.h"
#include "QCKey.h"
#include "QCObject.h"

typedef struct QCCipher * QCCipherRef;

extern QCCipherRef QCCipherCreate(void);

extern QCKeyRef QCCipherGetPrivateKey(QCCipherRef cipher);

extern QCKeyRef QCCipherGetPublicKey(QCCipherRef cipher);

extern void QCCipherSetPrivateKey(QCCipherRef cipher, QCKeyRef privateKey);

extern void QCCipherSetPublicKey(QCCipherRef cipher, QCKeyRef publicKey);

extern void QCCipherEncrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef *u, QCArrayRef *v);

extern QCArrayRef QCCipherSyndrome(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1);

extern QCArrayRef QCCipherDecrypt(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1);

extern QCArrayRef QCCipherEncryptMessage(QCCipherRef cipher, QCArrayRef message, QCKeyRef publicKey);

extern QCArrayRef QCCipherDecryptMessage(QCCipherRef cipher, QCArrayRef message, QCArrayRef rc_0, QCArrayRef rc_1);

/*
 * symmetric encrypt
 */
extern QCArrayRef QCCipherSymmetricEncrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef key, QCArrayRef iv);

/*
 * symmetric decrypt
 */
extern QCArrayRef QCCipherSymmetricDecrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef key, QCArrayRef iv);

#endif //PQC_CRYPTO_QCCIPHER_H
