//
// Created by Isaac on 2018/1/13.
//

#ifndef QN_QNBYTEARRAY_H
#define QN_QNBYTEARRAY_H

#include "QNArrayPrivate.h"

typedef void (* QNByteArrayLoopFunc)(QNByte value, size_t idx, const void *ctx);

const void *QNByteArrayCreate(const void *initData, size_t count, bool needCopy);

extern QN_STRONG QNArrayRef QNByteArrayFromFile(FILE *fp);

/*
 * create byte array from hex string
 */
extern QN_STRONG QNArrayRef QNByteArrayCreateWithHex(const char *hexString, size_t length);

/*
 * create byte array from base64 encoded string
 */
extern QN_STRONG QNArrayRef QNByteArrayCreateWithBase64(const char *base64String, size_t length);

/*
 * do pkcs7 encode on array
 */
extern QN_STRONG QNArrayRef QNByteArrayPKCS7Encode(QNArrayRef array);

/*
 * do pcks7 decode on array
 */
extern QN_STRONG QNArrayRef QNByteArrayPKCS7Decode(QNArrayRef array);

#endif //QN_QNBYTEARRAY_H
