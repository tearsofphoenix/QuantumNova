//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEY_H
#define PQC_CRYPTO_QCKEY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t length;
    size_t weight;
    size_t error;
} QCKeyConfig;

typedef struct QCKey *QCKeyRef;

extern QCKeyConfig kQCDefaultKeyConfig;

extern void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey);

extern QCKeyRef QCKeyCreateFromPEMFile(const char* filePath);

#endif //PQC_CRYPTO_QCKEY_H
