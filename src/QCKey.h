//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEY_H
#define PQC_CRYPTO_QCKEY_H

#include <stdbool.h>

typedef struct {
    int length;
    int weight;
    int error;
} QCKeyConfig;

typedef struct QCKey *QCKeyRef;

extern QCKeyRef QCKeyCreate(QCKeyConfig config, bool privateOrPublic);

extern void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey);

extern QCKeyRef QCKeyCreateFromPEMFile(const char* filePath);

#endif //PQC_CRYPTO_QCKEY_H
