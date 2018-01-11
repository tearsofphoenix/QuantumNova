//
// Created by Isaac on 2018/1/11.
//
#include "QCArrayPrivate.h"
#include "QCArray.h"
#include <printf.h>
#include <math.h>

void QCArrayPrint(QCArrayRef array) {
    int count = array->count;
    QCArrayDataType type = array->datatype;

    int padding = 25;
    printf("\n[ ");

    switch(type) {
        case QCDTInt: {
            const char* fmt = "%d, ";
            double *data = array->data;
            for (int i = 0; i < count; ++i) {
                printf(fmt, (int)data[i]);
                if (i % padding == 0 && i > 0) {
                    printf("\n");
                }
            }
            break;
        }
        case QCDTFloat:
        case QCDTDouble: {
            const char* fmt = "%f, ";
            double *data = array->data;
            for (int i = 0; i < count; ++i) {
                printf(fmt, data[i]);
                if (i % padding == 0 && i > 0) {
                    printf("\n");
                }
            }
            break;
        }
    }

    printf(" ]\n");
}

static bool _arrayCompare(const double *array, const double *expected, int count) {
    bool equal = true;
    if (array && expected) {
        int total = 0;
        for (int i = 0; i < count; ++i) {
            if (fabs(array[i] - expected[i]) > 0.00000005) {
                printf("not equal: %d %f %f\n", i, array[i], expected[i]);
                equal = false;
                ++total;
            }
        }
        if (!equal) {
            printf("total not equal: %d rate: %.2f%%", total, total * 100.0 / count);
        }
    }
    return equal;
}

bool QCArrayCompare(QCArrayRef xArray, QCArrayRef yArray) {
    int count = xArray->count;
    double *array = xArray->data;
    double *expected = yArray->data;
    return _arrayCompare(array, expected, count);
}

bool QCArrayCompareRaw(QCArrayRef x, const double *expected) {
    int count = x->count;
    double *array = x->data;
    return _arrayCompare(array, expected, count);
}
