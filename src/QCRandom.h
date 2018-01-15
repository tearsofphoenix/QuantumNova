//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCRANDOM_H
#define PQC_CRYPTO_QCRANDOM_H

#include "QCArray.h"

extern QCArrayRef QCRandomVector(size_t count);

extern QCArrayRef QCRandomWeightVector(size_t count, size_t weight);

extern int QCRandomFlipCoin();

#endif //PQC_CRYPTO_QCRANDOM_H
