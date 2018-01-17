//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCDOUBLEARRAY_H
#define PQC_CRYPTO_QCDOUBLEARRAY_H

#include "QCArrayPrivate.h"

typedef void (* QCDoubleLoopFunc)(double value, size_t idx, const void *ctx);

extern const QCClassRef kQCDoubleArrayClassRef;

extern QC_STRONG QCArrayRef QCDoubleArrayCreate(const void *initData, size_t count, bool needCopy);


#endif //PQC_CRYPTO_QCDOUBLEARRAY_H
