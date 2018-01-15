//
// Created by Isaac on 2018/1/13.
//

#ifndef PQC_CRYPTO_QCBYTEARRAY_H
#define PQC_CRYPTO_QCBYTEARRAY_H

#include "QCArrayPrivate.h"

typedef void (* QCByteArrayLoopFunc)(QCByte value, size_t idx, const void *ctx);

const void *QCByteArrayCreate(const void *initData, size_t count, bool needCopy);

extern QCArrayRef QCByteArrayCreateWithHex(const char *hexString, size_t length);

extern QCArrayRef QCByteArrayCreateWithBase64(const char *base64String, size_t length);

extern QCArrayRef QCByteArrayPKCS7Encode(QCArrayRef array);

extern QCArrayRef QCByteArrayPKCS7Decode(QCArrayRef array);

#endif //PQC_CRYPTO_QCBYTEARRAY_H
