//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCRANDOM_H
#define PQC_CRYPTO_QCRANDOM_H

#include "QCArray.h"

/*
 * vector of `count`, contains randomly 0/1
 */
extern QN_STRONG QCArrayRef QNRandomVector(size_t count);

/*
 * vector of `count`, `weight` of it is 1
 */
extern QN_STRONG QCArrayRef QNRandomWeightVector(size_t count, size_t weight);

/*
 * return 0 or 1
 */
extern int QNRandomFlipCoin();

#endif //PQC_CRYPTO_QCRANDOM_H
