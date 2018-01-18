//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCINT32ARRAY_H
#define PQC_CRYPTO_QCINT32ARRAY_H

#include "QCArrayPrivate.h"

typedef void (* QCInt32LoopFunc)(int value, size_t idx, const void *ctx);

const QC_STRONG QCArrayRef QCInt32ArrayCreate(const void *initData, size_t count, bool needCopy);

#endif //PQC_CRYPTO_QCINT32ARRAY_H
