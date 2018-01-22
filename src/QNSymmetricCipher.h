//
// Created by Isaac on 2018/1/22.
//

#ifndef QN_QNSYMMETRICCIPHER_H
#define QN_QNSYMMETRICCIPHER_H

#include "QNClass.h"
#include "QNArray.h"

typedef QN_STRONG QNArrayRef (* QNSymmetricEncryptFunc)(QNArrayRef plainText, QNArrayRef key, QNArrayRef iv);

typedef QN_STRONG QNArrayRef (* QNSymmetricDecryptFunc)(QNArrayRef data, QNArrayRef key, QNArrayRef iv);

struct QNSymmetricCipher {
    QNSymmetricEncryptFunc encrypt;
    QNSymmetricEncryptFunc decrypt;
};

typedef struct QNSymmetricCipher* QNSymmetricCipherRef;

extern QNSymmetricCipherRef QNGetAESCipher();

extern QNSymmetricCipherRef QNGetSalsa20Cipher();

#endif //QN_QNSYMMETRICCIPHER_H
