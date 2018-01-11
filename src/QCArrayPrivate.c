//
// Created by Isaac on 2018/1/11.
//
#include "QCArrayPrivate.h"
#include "QCArray.h"
#include <printf.h>
#include <math.h>
#include <fftw3.h>
#include <memory.h>

void *_QCMallocData(QCArrayDataType type, int count, size_t *outsize) {
    size_t size = 0;
    switch (type) {
        case QCDTInt: {
            size = sizeof(int) * count;
            break;
        }
        case QCDTFloat: {
            size = sizeof(float) * count;
            break;
        }
        case QCDTDouble: {
            size = sizeof(double) * count;
            break;
        }
    }
    void *data = fftw_malloc(size);
    memset(data, 0, size);

    if (outsize) {
        *outsize = size;
    }
    return data;
}

////////////////////////////////////////////////////////////////
//                base collection functions                   //
////////////////////////////////////////////////////////////////

void QCArraySetCount(QCArrayRef array, int newCount) {
    if (array) {
        array->count = newCount;
    }
}

void QCArraySetValueAt(QCArrayRef array, int index, double value) {
    if (array && index < array->count) {
        QCARRAYONE(array, d[index] = (int)value, d[index] = value);
    }
}

double QCArrayValueAt(QCArrayRef array, int index) {
    if (array) {
        QCARRAYONE(array, return d[index], return d[index]);
    }
    return 0;
}

int QCArrayGetNonZeroCount(QCArrayRef array) {
    if (array) {
        int total = 0;
        QCARRAYEACH(array, if (d[i] != 0) { ++total; }, if ((int)d[i] != 0) { ++total; });
        return total;
    }
    return 0;
}

void QCArraySetFFTFlag(QCArrayRef array, bool flag) {
    if (array) {
        array->fft = flag;
    }
}

QCArrayRef QCArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        int *indices = _QCMallocData(QCDTInt, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if ((int) round(x[i]) != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        int *result = NULL;
        if (idx > 0) {
            size_t size = 0;
            result = _QCMallocData(QCDTInt, idx, &size);
            memcpy(result, indices, size);
        }

        fftw_free(indices);

        QCArrayRef ref = QCArrayCreateNoCopy((void *)result, idx, true);
        ref->datatype = QCDTInt;
        return ref;
    }
    return NULL;
}

int QCArrayFindIndex(QCArrayRef array, int value) {
    if (array) {
        int count = array->count;
        int *data = array->data;
        for (int i = 0; i < count; ++i) {
            if (data[i] == value) {
                return i;
            }
        }
    }
    return -1;
}

////////////////////////////////////////////////////////////////
//                auxiliary functions                         //
////////////////////////////////////////////////////////////////

void QCArrayPrint(QCArrayRef array) {
    int count = array->count;
    QCArrayDataType type = array->datatype;

    int padding = 25;
    printf("\n[ ");

    QCARRAYEACH(array,
                printf("%d, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); },
                printf("%f, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); });

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

void QCArrayFixConjugateHalf(QCArrayRef array) {
    if (array) {
        int count = array->count;
        int half = count / 2;
        switch (array->datatype) {
            case QCDTInt: {
                int *d = array->data;
                for (int i = 0; i < half; ++i) {
                    printf("%d\n", count - i);
                    d[count - i] = d[i];
                }
                break;
            }
            default: {
                double *d = array->data;
                for (int i = 0; i < half; ++i) {
                    d[count - i] = d[i];
                }
                break;
            }
        }
    }
}
