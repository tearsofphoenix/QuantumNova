//
// Created by Isaac on 2018/1/22.
//

#ifndef PQC_CRYPTO_QNASYMMETRICCIPHER_H
#define PQC_CRYPTO_QNASYMMETRICCIPHER_H

#include "QNClass.h"
#include "QNArray.h"
#include "QNKey.h"

/*
 * do asymmetric encrypt on message, result: [u, v]
 */
typedef void (* QNAsymmetricEncryptFunc)(QNKeyRef publicKey, QNArrayRef random, QNArrayRef *u, QNArrayRef *v);

typedef void (* QNAsymmetricDecryptFunc)(QNKeyRef privateKey, QNArrayRef c0, QNArrayRef c1);

/*
 * asymmetric decrypt on c0 & c1
 */
typedef QN_STRONG QNArrayRef (* QNAsymmetricSyndromeFunc)(QNKeyRef privateKey, QNArrayRef c0, QNArrayRef c1);

struct QNAsymmetricCipher {
    QNAsymmetricEncryptFunc encrypt;
    QNAsymmetricDecryptFunc decrypt;
    QNAsymmetricSyndromeFunc syndrome;
};

typedef struct QNAsymmetricCipher* QNAsymmetricCipherRef;

extern QNAsymmetricCipherRef QNGetAsymmetricCipher();

#endif //PQC_CRYPTO_QNASYMMETRICCIPHER_H
