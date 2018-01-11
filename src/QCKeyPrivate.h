//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEYPRIVATE_H
#define PQC_CRYPTO_QCKEYPRIVATE_H

#include "QCArray.h"
#include "QCKey.h"

struct QCKey {
    // private key fields
    QCArrayRef h0;
    QCArrayRef h1;
    QCArrayRef h1inv;

    // public key fields
    QCArrayRef g;

    // shared fields
    int length;
    int weight;
    int error;
};

extern QCKeyRef QCKeyMalloc();

extern void QCKeyFree(QCKeyRef key);

#endif //PQC_CRYPTO_QCKEYPRIVATE_H
