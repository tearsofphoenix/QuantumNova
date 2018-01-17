//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCRANDOM_H
#define PQC_CRYPTO_QCRANDOM_H

#include "QCArray.h"

/*
 * vector of `count`, contains randomly 0/1
 */
extern QC_STRONG QCArrayRef QCRandomVector(size_t count);

/*
 * vector of `count`, `weight` of it is 1
 */
extern QC_STRONG QCArrayRef QCRandomWeightVector(size_t count, size_t weight);

/*
 * return 0 or 1
 */
extern int QCRandomFlipCoin();

#endif //PQC_CRYPTO_QCRANDOM_H
