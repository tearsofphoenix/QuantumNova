//
// Created by Isaac on 2018/1/12.
//

#include "QCInt32Array.h"
#include "QCArrayPrivate.h"
#include <memory.h>
#include <math.h>
#include <printf.h>
#include <fftw3.h>

static void QCInt32ArrayEnumerator(QCArrayRef array, const void *func, const void *ctx);
static const void *QCInt32ArrayCopy(QCArrayRef array);
static QCArrayRef QCInt32ArrayAdd (QCArrayRef x, QCArrayRef y);
static QCArrayRef QCInt32ArrayMultiply (QCArrayRef x, double mul);
static QCArrayRef QCInt32ArrayRound (QCArrayRef x);
static QCArrayRef QCInt32ArrayMod (QCArrayRef x, int mod);
static size_t QCInt32ArrayZeroCount (QCArrayRef x);
static void QCInt32ArrayAddAt(QCArrayRef x, int index, double value);
static void QCInt32ArrayXORAt(QCArrayRef x, int index, int value);
static void QCInt32ArraySetAt(QCArrayRef x, int index, double value);
static double QCInt32ArrayGetAt(QCArrayRef x, int index);
static void QCInt32ArrayPrint(QCArrayRef x);
static bool QCInt32ArrayEqual(QCArrayRef x, QCArrayRef y);
static double QCInt32ArrayMax(QCArrayRef array);
static QCArrayRef QCInt32ArrayRealParts(QCArrayRef x);
static QCArrayRef QCInt32ArrayComplexMultiply(QCArrayRef xArray, QCArrayRef yArray);
static QCArrayRef QCInt32ArrayGetNoZeroIndices(QCArrayRef array);
static QCArrayRef QCInt32ArraySHA256(QCArrayRef array);

static struct QCArrayClass kQCInt32ArrayClass = {
//        .base = kQCArrayClassRef,
        .name = "QCInt32Array",
        .allocator = QCAllocator,
        .size = sizeof(struct QCArray),
        .deallocate = QCArrayDeallocate,
        .copy = QCInt32ArrayCopy,
        .enumerator = QCInt32ArrayEnumerator,
        .add = QCInt32ArrayAdd,
        .multiply = QCInt32ArrayMultiply,
        .round = QCInt32ArrayRound,
        .mod = QCInt32ArrayMod,
        .zero = QCInt32ArrayZeroCount,
        .addAt = QCInt32ArrayAddAt,
        .xorAt = QCInt32ArrayXORAt,
        .set = QCInt32ArraySetAt,
        .get = QCInt32ArrayGetAt,
        .print = QCInt32ArrayPrint,
        .equal = QCInt32ArrayEqual,
        .max = QCInt32ArrayMax,
        .real = QCInt32ArrayRealParts,
        .complexMultiply = QCInt32ArrayComplexMultiply,
        .nonzeroIndices = QCInt32ArrayGetNoZeroIndices,
        .sha256 = QCInt32ArraySHA256
};

const QCClassRef kQCInt32ArrayClassRef = &kQCInt32ArrayClass;

const void *QCInt32ArrayCreate(const void *initData, size_t count, bool needCopy) {
    if (count > 0) {
        QCArrayDataType type = QCDTInt;
        QCArrayRef array = QCAllocate(&kQCInt32ArrayClass);
        array->isa = kQCInt32ArrayClassRef;

        if (initData) {
            if (needCopy) {
                size_t size = 0;
                array->data = _QCMallocData(type, count, &size);
                memcpy(array->data, initData, size);
            } else {
                array->data = initData;
            }
        } else {
            array->data = _QCMallocData(type, count, NULL);
        }
        array->count = count;
        array->fft = false;
        array->needfree = needCopy;
        array->datatype = type;
        return array;
    }
    return NULL;
}

static const void *QCInt32ArrayCopy(QCArrayRef array) {
    if (array) {
        return QCInt32ArrayCreate(array->data, array->count, true);
    }
    return NULL;
}


static void QCInt32ArrayEnumerator(QCArrayRef array, const void *func, const void *ctx) {
    QCInt32LoopFunc f = func;
    QCFOREACH(array, f(d[i], i, ctx), int);
}

static QCArrayRef QCInt32ArrayAdd (QCArrayRef array, QCArrayRef y) {
    if (y->isa == kQCInt32ArrayClassRef) {
        // both int
        int *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], int);
    } else {
        double *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], int);
    }
    return array;
}

static QCArrayRef QCInt32ArrayMultiply (QCArrayRef array, double mul) {
    int m = (int)mul;
    QCFOREACH(array, d[i] *= m, int);
}
static QCArrayRef QCInt32ArrayRound (QCArrayRef array) {
    // no need to round
    return array;
}
static QCArrayRef QCInt32ArrayMod (QCArrayRef array, int mod) {
    QCFOREACH(array, d[i] = d[i] % mod, int);
}

static size_t QCInt32ArrayZeroCount (QCArrayRef array) {
    size_t total = 0;
    QCFOREACH(array, if (d[i] == 0){ ++total; }, int);
    return total;
}

QCARRAYIMP(QCInt32Array, int)

static void QCInt32ArrayPrint(QCArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x>[ ", array->isa->name, array);

        array->isa->print(array);
        QCFOREACH(array, printf("%d, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, int);

        printf(" ]\n");
    }
}

static bool QCInt32ArrayEqual(QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTInt) {
        return memcmp(array->data, y->data, array->count * sizeof(size_t)) == 0;
    } else {
        double *dy = y->data;
        QCFOREACH(array, if (d[i] != (int)dy[i]) { return false; }, int);
        return true;
    }
}

static double QCInt32ArrayMax(QCArrayRef array) {
    int max = 0;
    QCFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, int);
    return max;
}

static QCArrayRef QCInt32ArrayRealParts(QCArrayRef x) {
    if (x) {
        int count = x->count;
        if (x->fft) {
            count -= 2;
        }
        int *array = x->data;
        double *out = x->isa->allocator(count * sizeof(double));
        for (int i = 0; i < count; i = i + 2) {
            double temp = array[i];
            out[i / 2] = temp;
            out[count - i / 2] = temp;
        }
        return QCArrayCreateWithDouble(out, count, true);
    }
    return NULL;
}


static QCArrayRef QCInt32ArrayComplexMultiply(QCArrayRef xArray, QCArrayRef yArray) {
    if (yArray->datatype == QCDTInt) {
        // both double
        int count = xArray->count;
        int *x = xArray->data;
        int *y = yArray->data;
        double *out = xArray->isa->allocator(count * sizeof(double));
        for (int i = 0; i < count; i = i + 2) {
            double a = x[i];
            double b = x[i + 1];
            double c = y[i];
            double d = y[i + 1];
            out[i] = a * c - b * d;
            out[i + 1] = a * d + b * c;
        }
        if (count % 2 == 1) {
            out[count - 1] = x[count - 1] * y[count - 1];
        }
        return QCArrayCreateWithDouble(out, count, true);
    } else {
        int count = xArray->count;
        int *x = xArray->data;
        double *y = yArray->data;
        double *out = xArray->isa->allocator(count * sizeof(double));
        for (int i = 0; i < count; i = i + 2) {
            double a = x[i];
            double b = x[i + 1];
            double c = y[i];
            double d = y[i + 1];
            out[i] = a * c - b * d;
            out[i + 1] = a * d + b * c;
        }
        if (count % 2 == 1) {
            out[count - 1] = x[count - 1] * y[count - 1];
        }
        return QCArrayCreateWithDouble(out, count, true);
    }
}

static QCArrayRef QCInt32ArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        size_t count = array->count;
        int *x = array->data;
        int *indices = _QCMallocData(QCDTInt, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if (x[i] != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        QCArrayRef ref = QCArrayCreateWithInt(indices, idx, true);
        fftw_free(indices);
        return ref;
    }
    return NULL;
}

static QCByte *_int32ArrayToCharArray(int *array, size_t count) {

}

static QCArrayRef QCInt32ArraySHA256(QCArrayRef array) {
    if (array) {

    }
    return NULL;
}