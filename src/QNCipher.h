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
 * encrypt message with publicKey
 */
extern QN_STRONG QNMessageRef QNCipherEncryptMessage(QNCipherRef cipher, QNArrayRef plainData);

/*
 * decrypt message with privateKey
 */
extern QN_STRONG QNArrayRef QNCipherDecryptMessage(QNCipherRef cipher, QNMessageRef message);

#endif //QN_QNCIPHER_H
