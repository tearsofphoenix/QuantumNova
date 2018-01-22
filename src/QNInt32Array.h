//
// Created by Isaac on 2018/1/12.
//

#ifndef QN_QNINT32ARRAY_H
#define QN_QNINT32ARRAY_H

#include "QNArrayPrivate.h"

typedef void (* QNInt32LoopFunc)(int value, size_t idx, const void *ctx);

extern const QN_STRONG QNArrayRef QNInt32ArrayCreate(const void *initData, size_t count, bool needCopy);

extern QN_STRONG QNArrayRef QNInt32ArrayFromFile(FILE *fp);

#endif //QN_QNINT32ARRAY_H
