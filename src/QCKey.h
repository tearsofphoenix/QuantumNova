//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEY_H
#define PQC_CRYPTO_QCKEY_H

#include <stdbool.h>
#include <stddef.h>
#include "QCClass.h"

typedef struct {
    size_t length;
    size_t weight;
    size_t error;
} QCKeyConfig;

typedef struct QCKey *QCKeyRef;

/*
 * default key config
 */
extern QCKeyConfig kQCDefaultKeyConfig;

/*
 * 128bit security key config
 */
extern QCKeyConfig kQC128BitKeyConfig;

/*
 * 256bit security key config
 */
extern QCKeyConfig kQC256BitKeyConfig;

/*
 * generate private & public key pair
 */
extern void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey);

/*
 * load key from PEM format file
 */
extern QN_STRONG QCKeyRef QCKeyCreateFromPEMFile(const char* filePath);

/*
 * save key to file (PEM format)
 */
extern void QCKeySaveToPEMFile(QCKeyRef key, const char *path);

#endif //PQC_CRYPTO_QCKEY_H
