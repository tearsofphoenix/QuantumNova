//
// Created by Isaac on 2018/1/11.
//

#ifndef QN_QNRANDOM_H
#define QN_QNRANDOM_H

#include "QNArray.h"

/*
 * vector of `count`, contains randomly 0/1
 */
extern QN_STRONG QNArrayRef QNRandomVector(size_t count);

/*
 * vector of `count`, `weight` of it is 1
 */
extern QN_STRONG QNArrayRef QNRandomWeightVector(size_t count, size_t weight);

/*
 * return 0 or 1
 */
extern int QNRandomFlipCoin();

#endif //QN_QNRANDOM_H
