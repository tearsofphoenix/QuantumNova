//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCKEYPRIVATE_H
#define PQC_CRYPTO_QCKEYPRIVATE_H

#include "QCArray.h"
#include "QCKey.h"
#include "QCObject.h"
#include <tomcrypt.h>

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
extern QC_STRONG QCKeyRef QCKeyCreatePrivate(QCArrayRef h0, QCArrayRef h1, QCArrayRef h1inv, QCKeyConfig config);

/*
 * create public key from arrays
 */
extern QC_STRONG QCKeyRef QCKeyCreatePublic(QCArrayRef g, QCKeyConfig config);


extern bool _isKindOfFile(const char *fileContent, const char *label);

extern QC_STRONG QCByte *_trimFileContent(const char *fileContent, size_t fileLength, size_t *outLength, const char *label);

extern QC_STRONG QCByte *_readFileContent(const char *path, size_t *outLength);

extern QC_STRONG QCArrayRef _decodeBitString(ltc_asn1_list *node);

extern QC_STRONG QCArrayRef _decodeOCTString(ltc_asn1_list *node);

#endif //PQC_CRYPTO_QCKEYPRIVATE_H
