//
// Created by Isaac on 2018/1/11.
//

#include <stdlib.h>
#include <math.h>
#include "QCRandom.h"

static int getRandomInt(int min, int max) {
    return (int)floor(random() * (max - min)) + min; //The maximum is exclusive and the minimum is inclusive
}


QCArrayRef QCRandomVector(int count) {
    QCArrayRef vector = QCArrayCreate(count);
    for (int i = 0; i < count; ++i) {
        QCArraySetValueAt(vector, i, getRandomInt(0, 2));
    }
    return vector;
}

QCArrayRef QCRandomWeightVector(int count, int weight) {
    QCArrayRef random_indices = QCArrayCreateWithType(QCDTInt, weight);
    while (QCArrayGetNonZeroCount(random_indices) < weight) {
        int ind = QCArrayGetNonZeroCount(random_indices);
        int rnd = getRandomInt(0, count);
        if (QCArrayFindIndex(random_indices, rnd) == -1) {
            QCArraySetValueAt(random_indices, ind, rnd);
        }
    }

    QCArrayRef real = QCArrayCreate(count);
    for(int i = 0; i < weight; ++i) {
        int idx = QCArrayValueAt(random_indices, i);
        QCArraySetValueAt(real, idx, 1);
    }

    QCArrayFree(random_indices);

    return real;
}
