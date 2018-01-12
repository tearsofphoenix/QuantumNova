//
// Created by Isaac on 2018/1/11.
//
#include "QCArrayPrivate.h"
#include "QCArray.h"
#include <printf.h>
#include <math.h>
#include <fftw3.h>
#include <memory.h>

static struct QCClass kQCArrayClass = {
        .base = NULL,
        .name = "QCClass",
        .allocator = QCAllocator,
        .deallocate = QCDeallocate,
        .copy = NULL
};

const QCClassRef kQCArrayClassRef = &kQCArrayClass;

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

void QCArraySetCount(QCArrayRef array, size_t newCount) {
    if (array) {
        array->count = newCount;
    }
}

void QCArraySetValueAt(QCArrayRef array, int index, double value) {
    if (array && index < array->count) {
        ((QCArrayClassRef)array->isa)->set(array, index, value);
    }
}

double QCArrayValueAt(QCArrayRef array, int index) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->get(array, index);
    }
    return 0;
}

int QCArrayGetNonZeroCount(QCArrayRef array) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->zero(array);
    }
    return 0;
}

void QCArrayForeach(QCArrayRef array, const void* func, const void *ctx) {
    if (array && func) {
        ((QCArrayClassRef)array->isa)->enumerator(array, func, ctx);
    }
}

double QCArrayMax(QCArrayRef array) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->max(array);
    }
    return 0;
}

QCArrayRef QCArrayGetRealParts(QCArrayRef complexArray) {
    if (complexArray) {
        return ((QCArrayClassRef)complexArray->isa)->real(complexArray);
    }
    return NULL;
}

void QCArrayAddArray(QCArrayRef x, QCArrayRef y) {
    if (x && y) {
        ((QCArrayClassRef)x->isa)->add(x, y);
    }
}

void QCArrayMultiply(QCArrayRef array, double mul) {
    if (array) {
        ((QCArrayClassRef)array->isa)->multiply(array, mul);
    }
}

void QCArrayRound(QCArrayRef array) {
    if (array) {
        ((QCArrayClassRef)array->isa)->round(array);
    }
}

void QCArrayAddAt(QCArrayRef array, int index, double value) {
    if (array && index < array->count) {
        ((QCArrayClassRef)array->isa)->addAt(array, index, value);
    }
}

void QCArrayMod(QCArrayRef array, int mod) {
    if (array) {
        ((QCArrayClassRef)array->isa)->mod(array, mod);
    }
}

QCArrayRef QCArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->nonzeroIndices(array);
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

static bool _arrayCompareDouble(const double *array, const double *expected, int count) {
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

static bool _arrayCompareMix(const int *array, const double *expected, int count) {
    bool equal = true;
    if (array && expected) {
        int total = 0;
        for (int i = 0; i < count; ++i) {
            if (fabs(array[i] - expected[i]) > 0.00000005) {
                printf("not equal: %d %d %f\n", i, array[i], expected[i]);
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

static bool _arrayCompareInt(const int *array, const int *expected, int count) {
    bool equal = true;
    if (array && expected) {
        int total = 0;
        for (int i = 0; i < count; ++i) {
            if (array[i] != expected[i]) {
                printf("not equal: %d %d %d\n", i, array[i], expected[i]);
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

bool QCArrayCompareRaw(QCArrayRef x, const double *expected) {
    if (x && expected) {
        int count = x->count;
        if (x->datatype == QCDTInt) {
            return _arrayCompareMix(x->data, expected, count);
        } else {
            return _arrayCompareDouble(x->data, expected, count);
        }
    }
    return true;
}

void QCArrayXORAt(QCArrayRef array, int index, int value) {
    if (array) {
        ((QCArrayClassRef)array->isa)->xorAt(array, index, value);
    }
}

void QCArrayDeallocate(QCArrayRef array) {
    if (array && array->needfree) {
        fftw_free(array->data);
    }
}
