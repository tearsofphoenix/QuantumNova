//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCCIPHER_H
#define PQC_CRYPTO_QCCIPHER_H

#include "QCArray.h"
#include "QCKey.h"
#include "QCObject.h"
#include "QCMessage.h"

typedef struct QCCipher * QCCipherRef;

/*
 * create empty cipher
 */
extern QCCipherRef QCCipherCreate(void);

/*
 * get current private key of cipher
 */
extern QCKeyRef QCCipherGetPrivateKey(QCCipherRef cipher);

/*
 * get current public key of cipher
 */
extern QCKeyRef QCCipherGetPublicKey(QCCipherRef cipher);

/*
 * set current private key of cipher, will retain privateKey
 */
extern void QCCipherSetPrivateKey(QCCipherRef cipher, QCKeyRef privateKey);

/*
 * set current public key of cipher, will retain publicKey
 */
extern void QCCipherSetPublicKey(QCCipherRef cipher, QCKeyRef publicKey);

/*
 * do asymmetric encrypt on message, result: [u, v]
 */
extern void QCCipherEncrypt(QCCipherRef cipher, QCArrayRef random, QCArrayRef *u, QCArrayRef *v);

/*
 * syndrome c0 & c1
 */
extern QN_STRONG QCArrayRef QCCipherSyndrome(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1);

/*
 * asymmetric decrypt on c0 & c1
 */
extern void QCCipherDecrypt(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1);

/*
 * encrypt message with publicKey
 */
extern QN_STRONG QCMessageRef QCCipherEncryptMessage(QCCipherRef cipher, QCArrayRef plainData);

/*
 * decrypt message with privateKey
 */
extern QN_STRONG QCArrayRef QCCipherDecryptMessage(QCCipherRef cipher, QCMessageRef message);

#endif //PQC_CRYPTO_QCCIPHER_H
