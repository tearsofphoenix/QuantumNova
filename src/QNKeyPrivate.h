//
// Created by Isaac on 2018/1/11.
//

#ifndef QN_QNKEYPRIVATE_H
#define QN_QNKEYPRIVATE_H

#include "QNArray.h"
#include "QNKey.h"
#include "QNObject.h"
#include <tomcrypt.h>

struct QNKey {
    QNOBJECTFIELDS
    // private key fields
    QNArrayRef h0;
    QNArrayRef h1;
    QNArrayRef h1inv;

    // public key fields
    QNArrayRef g;

    // shared fields
    size_t length;
    size_t weight;
    size_t error;
};

/*
 * create private key from arrays
 */
extern QN_STRONG QNKeyRef QNKeyCreatePrivate(QNArrayRef h0, QNArrayRef h1, QNArrayRef h1inv, QNKeyConfig config);

/*
 * create public key from arrays
 */
extern QN_STRONG QNKeyRef QNKeyCreatePublic(QNArrayRef g, QNKeyConfig config);


extern bool _isKindOfFile(const char *fileContent, const char *label);

extern QN_STRONG QNByte *_trimFileContent(const char *fileContent, size_t fileLength, size_t *outLength, const char *label);

extern QN_STRONG QNByte *_readFileContent(const char *path, size_t *outLength);

extern QN_STRONG QNArrayRef _decodeBitString(ltc_asn1_list *node);

extern QN_STRONG QNArrayRef _decodeOCTString(ltc_asn1_list *node);

/*
 * save key to file (base64 format)
 */
extern bool QNKeySaveToFile(QNKeyRef key, const char *path);

/*
 * load key from file (base64 format)
 */
extern QNKeyRef QNKeyCreateFromFile(const char *path);

#endif //QN_QNKEYPRIVATE_H
