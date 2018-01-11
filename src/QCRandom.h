//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCRANDOM_H
#define PQC_CRYPTO_QCRANDOM_H

#include "QCArray.h"

extern QCArrayRef QCRandomVector(int count);

extern QCArrayRef QCRandomWeightVector(int count, int weight);

#endif //PQC_CRYPTO_QCRANDOM_H
