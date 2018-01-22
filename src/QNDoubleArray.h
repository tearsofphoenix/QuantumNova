//
// Created by Isaac on 2018/1/12.
//

#ifndef QN_QNDOUBLEARRAY_H
#define QN_QNDOUBLEARRAY_H

#include "QNArrayPrivate.h"

typedef void (* QNDoubleLoopFunc)(double value, size_t idx, const void *ctx);

extern const QNClassRef kQNDoubleArrayClassRef;

extern QN_STRONG QNArrayRef QNDoubleArrayCreate(const void *initData, size_t count, bool needCopy);

extern QN_STRONG QNArrayRef QNDoubleArrayFromFile(FILE *fp);

#endif //QN_QNDOUBLEARRAY_H
