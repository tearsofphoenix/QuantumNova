//
// Created by Isaac on 2018/1/11.
//
#include "QCArrayPrivate.h"
#include <printf.h>
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

void *_QCMallocData(QCArrayDataType type, size_t count, size_t *outsize) {
    size_t size = 0;
    switch (type) {
        case QCDTInt: {
            size = sizeof(int) * count;
            break;
        }
        case QCDTByte: {
            size = sizeof(QCByte) * count;
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
        return array->count - ((QCArrayClassRef)array->isa)->zero(array);
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

bool QCArrayCompareRaw(QCArrayRef x, const void *expected, QCArrayDataType dataType) {
    if (x && expected) {
        return ((QCArrayClassRef)x->isa)->compareRaw(x, expected, dataType);
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
