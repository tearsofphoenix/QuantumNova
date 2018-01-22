//
// Created by Isaac on 2018/1/22.
//

#ifndef PQC_CRYPTO_QNSYMMETRICCIPHER_H
#define PQC_CRYPTO_QNSYMMETRICCIPHER_H

#include "QCClass.h"
#include "QCArray.h"

typedef QN_STRONG QCArrayRef (* QNSymmetricEncryptFunc)(QCArrayRef plainText, QCArrayRef key, QCArrayRef iv);

typedef QN_STRONG QCArrayRef (* QNSymmetricDecryptFunc)(QCArrayRef data, QCArrayRef key, QCArrayRef iv);

struct QNSymmetricCipher {
    QNSymmetricEncryptFunc encrypt;
    QNSymmetricEncryptFunc decrypt;
};

typedef struct QNSymmetricCipher* QNSymmetricCipherRef;

extern QNSymmetricCipherRef QNGetAESCipher();

extern QNSymmetricCipherRef QNGetSalsa20Cipher();

#endif //PQC_CRYPTO_QNSYMMETRICCIPHER_H
