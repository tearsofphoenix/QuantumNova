//
// Created by Isaac on 2018/1/10.
//

#include "helper.h"
#include <stdio.h>
#include <math.h>

void QCPrintDoubleArray(double *array, int count) {
    int padding = 25;
    printf("\n[ ");
    for (int i = 0; i < count; ++i) {
        printf("%f, ", array[i]);
        if (i % padding == 0 && i > 0) {
            printf("\n");
        }
    }
    printf(" ]\n");
}

bool QCCompareArray(double *array, double *expected, int count) {
    bool equal = true;
    for (int i = 0; i < count; ++i) {
        if (fabs(array[i] - expected[i]) > 0.00000005) {
            printf("not equal: %d %f %f\n", i, array[i], expected[i]);
            equal = false;
        }
    }
    return equal;
}
