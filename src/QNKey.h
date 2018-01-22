//
// Created by Isaac on 2018/1/11.
//

#ifndef QN_QNKEY_H
#define QN_QNKEY_H

#include <stdbool.h>
#include <stddef.h>
#include "QNClass.h"

typedef struct {
    size_t length;
    size_t weight;
    size_t error;
} QNKeyConfig;

typedef struct QNKey *QNKeyRef;

/*
 * default key config
 */
extern QNKeyConfig kQNDefaultKeyConfig;

/*
 * 128bit security key config
 */
extern QNKeyConfig kQN128BitKeyConfig;

/*
 * 256bit security key config
 */
extern QNKeyConfig kQN256BitKeyConfig;

/*
 * generate private & public key pair
 */
extern void QNKeyGeneratePair(QNKeyConfig config, QNKeyRef *privateKey, QNKeyRef *publicKey);

/*
 * load key from PEM format file
 */
extern QN_STRONG QNKeyRef QNKeyCreateFromPEMFile(const char* filePath);

/*
 * save key to file (PEM format)
 */
extern void QNKeySaveToPEMFile(QNKeyRef key, const char *path);

#endif //QN_QNKEY_H
