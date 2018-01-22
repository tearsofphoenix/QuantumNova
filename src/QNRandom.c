//
// Created by Isaac on 2018/1/11.
//

#include <stdlib.h>
#include "QNRandom.h"
#include "QCObject.h"

static int getRandomInt(int min, int max) {
    int delta = max - min;
    return (int)random() % delta + min; //The maximum is exclusive and the minimum is inclusive
}

QCArrayRef QNRandomVector(size_t count) {
    QCArrayRef vector = QCArrayCreate(count);
    for (int i = 0; i < count; ++i) {
        QCArraySetValueAt(vector, i, getRandomInt(0, 2));
    }
    return vector;
}

QCArrayRef QNRandomWeightVector(size_t count, size_t weight) {
    QCArrayRef random_indices = QCArrayCreateWithInt(NULL, weight, false);
    while (QCArrayGetNonZeroCount(random_indices) < weight) {
        int ind = QCArrayGetNonZeroCount(random_indices);
        int rnd = getRandomInt(0, count);
        if (QCArrayFindIndex(random_indices, rnd) == -1) {
            QCArraySetValueAt(random_indices, ind, rnd);
        }
    }

    QCArrayRef real = QCArrayCreate(count);
    for(size_t i = 0; i < weight; ++i) {
        size_t idx = QCArrayValueAt(random_indices, i);
        QCArraySetValueAt(real, idx, 1);
    }

    QCRelease(random_indices);

    return real;
}

int QNRandomFlipCoin() {
    return getRandomInt(0, 2);
}
