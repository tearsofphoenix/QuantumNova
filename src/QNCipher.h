//
// Created by Isaac on 2018/1/11.
//

#ifndef QN_QNCIPHER_H
#define QN_QNCIPHER_H

#include "QNArray.h"
#include "QNKey.h"
#include "QNObject.h"
#include "QNMessage.h"

typedef struct QNCipher * QNCipherRef;

/*
 * create empty cipher
 */
extern QNCipherRef QNCipherCreate(void);

/*
 * get current private key of cipher
 */
extern QNKeyRef QNCipherGetPrivateKey(QNCipherRef cipher);

/*
 * get current public key of cipher
 */
extern QNKeyRef QNCipherGetPublicKey(QNCipherRef cipher);

/*
 * set current private key of cipher, will retain privateKey
 */
extern void QNCipherSetPrivateKey(QNCipherRef cipher, QNKeyRef privateKey);

/*
 * set current public key of cipher, will retain publicKey
 */
extern void QNCipherSetPublicKey(QNCipherRef cipher, QNKeyRef publicKey);

/*
 * do asymmetric encrypt on message, result: [u, v]
 */
extern void QNCipherEncrypt(QNCipherRef cipher, QNArrayRef random, QNArrayRef *u, QNArrayRef *v);

/*
 * syndrome c0 & c1
 */
extern QN_STRONG QNArrayRef QNCipherSyndrome(QNCipherRef cipher, QNArrayRef c0, QNArrayRef c1);

/*
 * asymmetric decrypt on c0 & c1
 */
extern void QNCipherDecrypt(QNCipherRef cipher, QNArrayRef c0, QNArrayRef c1);

/*
 * encrypt message with publicKey
 */
extern QN_STRONG QNMessageRef QNCipherEncryptMessage(QNCipherRef cipher, QNArrayRef plainData);

/*
 * decrypt message with privateKey
 */
extern QN_STRONG QNArrayRef QNCipherDecryptMessage(QNCipherRef cipher, QNMessageRef message);

#endif //QN_QNCIPHER_H
