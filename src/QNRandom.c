//
// Created by Isaac on 2018/1/11.
//

#include <stdlib.h>
#include "QNRandom.h"
#include "QNObject.h"

static int getRandomInt(int min, int max) {
    int delta = max - min;
    return (int)random() % delta + min; //The maximum is exclusive and the minimum is inclusive
}

QNArrayRef QNRandomVector(size_t count) {
    QNArrayRef vector = QNArrayCreate(count);
    for (int i = 0; i < count; ++i) {
        QNArraySetValueAt(vector, i, getRandomInt(0, 2));
    }
    return vector;
}

QNArrayRef QNRandomWeightVector(size_t count, size_t weight) {
    QNArrayRef random_indices = QNArrayCreateWithInt(NULL, weight, false);
    while (QNArrayGetNonZeroCount(random_indices) < weight) {
        int ind = QNArrayGetNonZeroCount(random_indices);
        int rnd = getRandomInt(0, count);
        if (QNArrayFindIndex(random_indices, rnd) == -1) {
            QNArraySetValueAt(random_indices, ind, rnd);
        }
    }

    QNArrayRef real = QNArrayCreate(count);
    for(size_t i = 0; i < weight; ++i) {
        size_t idx = QNArrayValueAt(random_indices, i);
        QNArraySetValueAt(real, idx, 1);
    }

    QNRelease(random_indices);

    return real;
}

int QNRandomFlipCoin() {
    return getRandomInt(0, 2);
}
