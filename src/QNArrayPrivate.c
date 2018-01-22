//
// Created by Isaac on 2018/1/11.
//
#include "QNArrayPrivate.h"
#include <fftw3.h>
#include <memory.h>

static struct QNClass kQNArrayClass = {
        .base = NULL,
        .name = "QNArray",
        .allocator = QNAllocator,
        .deallocate = QNDeallocate,
        .copy = NULL
};

void *_QNMallocData(QNArrayDataType type, size_t count, size_t *outsize) {
    size_t size = 0;
    switch (type) {
        case QNDTInt: {
            size = sizeof(int) * count;
            break;
        }
        case QNDTByte: {
            size = sizeof(QNByte) * count;
            break;
        }
        case QNDTDouble: {
            size = sizeof(double) * count;
            break;
        }
    }
    void *data = QNAllocator(size);
    memset(data, 0, size);

    if (outsize) {
        *outsize = size;
    }
    return data;
}

////////////////////////////////////////////////////////////////
//                base collection functions                   //
////////////////////////////////////////////////////////////////

void QNArraySetCount(QNArrayRef array, size_t newCount) {
    if (array) {
        array->count = newCount;
    }
}

void QNArraySetValueAt(QNArrayRef array, int index, double value) {
    if (array && index < array->count) {
        ((QNArrayClassRef)array->isa)->set(array, index, value);
    }
}

double QNArrayValueAt(QNArrayRef array, int index) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->get(array, index);
    }
    return 0;
}

int QNArrayGetNonZeroCount(QNArrayRef array) {
    if (array) {
        return (int)(array->count - ((QNArrayClassRef)array->isa)->zero(array));
    }
    return 0;
}

void QNArrayForeach(QNArrayRef array, const void* func, const void *ctx) {
    if (array && func) {
        ((QNArrayClassRef)array->isa)->enumerator(array, func, ctx);
    }
}

double QNArrayMax(QNArrayRef array) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->max(array);
    }
    return 0;
}

QNArrayRef QNArrayGetRealParts(QNArrayRef complexArray) {
    if (complexArray) {
        return ((QNArrayClassRef)complexArray->isa)->real(complexArray);
    }
    return NULL;
}

void QNArrayAddArray(QNArrayRef x, QNArrayRef y) {
    if (x && y) {
        ((QNArrayClassRef)x->isa)->add(x, y);
    }
}

void QNArrayMultiply(QNArrayRef array, double mul) {
    if (array) {
        ((QNArrayClassRef)array->isa)->multiply(array, mul);
    }
}

void QNArrayRound(QNArrayRef array) {
    if (array) {
        ((QNArrayClassRef)array->isa)->round(array);
    }
}

void QNArrayAddAt(QNArrayRef array, int index, double value) {
    if (array && index < array->count) {
        ((QNArrayClassRef)array->isa)->addAt(array, index, value);
    }
}

void QNArrayMod(QNArrayRef array, int mod) {
    if (array) {
        ((QNArrayClassRef)array->isa)->mod(array, mod);
    }
}

QNArrayRef QNArrayGetNoZeroIndices(QNArrayRef array) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->nonzeroIndices(array);
    }
    return NULL;
}

int QNArrayFindIndex(QNArrayRef array, int value) {
    if (array) {
        size_t count = array->count;
        int *data = array->data;
        for (size_t i = 0; i < count; ++i) {
            if (data[i] == value) {
                return (int)i;
            }
        }
    }
    return -1;
}

////////////////////////////////////////////////////////////////
//                auxiliary functions                         //
////////////////////////////////////////////////////////////////

bool QNArrayCompareRaw(QNArrayRef x, const void *expected, QNArrayDataType dataType) {
    if (x && expected) {
        return ((QNArrayClassRef)x->isa)->compareRaw(x, expected, dataType);
    }
    return false;
}

void QNArrayXORAt(QNArrayRef array, int index, int value) {
    if (array) {
        ((QNArrayClassRef)array->isa)->xorAt(array, index, value);
    }
}

void QNArrayDeallocate(QNArrayRef array) {
    if (array) {
        if (array->needfree) {
            QNDeallocate(array->data);
        }
        QNDeallocate(array);
    }
}
