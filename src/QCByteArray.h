//
// Created by Isaac on 2018/1/13.
//

#ifndef PQC_CRYPTO_QCBYTEARRAY_H
#define PQC_CRYPTO_QCBYTEARRAY_H

#include "QCArrayPrivate.h"

typedef void (* QCByteArrayLoopFunc)(QCByte value, size_t idx, const void *ctx);

const void *QCByteArrayCreate(const void *initData, size_t count, bool needCopy);

extern QC_STRONG QCArrayRef QCByteArrayFromFile(FILE *fp);

/*
 * create byte array from hex string
 */
extern QC_STRONG QCArrayRef QCByteArrayCreateWithHex(const char *hexString, size_t length);

/*
 * create byte array from base64 encoded string
 */
extern QC_STRONG QCArrayRef QCByteArrayCreateWithBase64(const char *base64String, size_t length);

/*
 * do pkcs7 encode on array
 */
extern QC_STRONG QCArrayRef QCByteArrayPKCS7Encode(QCArrayRef array);

/*
 * do pcks7 decode on array
 */
extern QC_STRONG QCArrayRef QCByteArrayPKCS7Decode(QCArrayRef array);

#endif //PQC_CRYPTO_QCBYTEARRAY_H
