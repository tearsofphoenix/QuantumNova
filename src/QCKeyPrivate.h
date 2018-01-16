//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEYPRIVATE_H
#define PQC_CRYPTO_QCKEYPRIVATE_H

#include "QCArray.h"
#include "QCKey.h"
#include "QCObject.h"

struct QCKey {
    QCOBJECTFIELDS
    // private key fields
    QCArrayRef h0;
    QCArrayRef h1;
    QCArrayRef h1inv;

    // public key fields
    QCArrayRef g;

    // shared fields
    size_t length;
    size_t weight;
    size_t error;
};

/*
 * create private key from arrays
 */
extern QCKeyRef QCKeyCreatePrivate(QCArrayRef h0, QCArrayRef h1, QCArrayRef h1inv, QCKeyConfig config);

/*
 * create public key from arrays
 */
extern QCKeyRef QCKeyCreatePublic(QCArrayRef g, QCKeyConfig config);

#endif //PQC_CRYPTO_QCKEYPRIVATE_H
