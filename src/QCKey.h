//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEY_H
#define PQC_CRYPTO_QCKEY_H

#include <stdbool.h>

typedef struct QCKey *QCKeyRef;

extern QCKeyRef QCKeyCreate(int length, int weight, bool privateOrPublic);

extern QCKeyRef QCKeyCreateFromPEMFile(const char* filePath);

#endif //PQC_CRYPTO_QCKEY_H
