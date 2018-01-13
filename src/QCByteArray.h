//
// Created by Isaac on 2018/1/13.
//

#ifndef PQC_CRYPTO_QCBYTEARRAY_H
#define PQC_CRYPTO_QCBYTEARRAY_H

#include "QCArrayPrivate.h"

typedef void (* QCByteArrayLoopFunc)(QCByte value, size_t idx, const void *ctx);

const void *QCByteArrayCreate(const void *initData, size_t count, bool needCopy);

#endif //PQC_CRYPTO_QCBYTEARRAY_H
