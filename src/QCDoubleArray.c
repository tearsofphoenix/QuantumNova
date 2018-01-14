//
// Created by Isaac on 2018/1/12.
//

#include <memory.h>
#include <math.h>
#include "QCDoubleArray.h"
#include "QCArrayPrivate.h"
#include "QCClass.h"
#include <printf.h>
#include <fftw3.h>

static const void *QCDoubleArrayCopy(QCArrayRef array);

static void QCDoubleArrayEnumerator(QCArrayRef array, const void *func, const void *ctx);
static QCArrayRef QCDoubleArrayAdd (QCArrayRef x, QCArrayRef y);
static QCArrayRef QCDoubleArrayMultiply (QCArrayRef x, double mul);
static QCArrayRef QCDoubleArrayRound (QCArrayRef x);
static QCArrayRef QCDoubleArrayMod (QCArrayRef x, int mod);
static size_t QCDoubleArrayZeroCount (QCArrayRef x);
static void QCDoubleArrayAddAt(QCArrayRef x, int index, double value);
static void QCDoubleArrayXORAt(QCArrayRef x, int index, int value);
static void QCDoubleArraySetAt(QCArrayRef x, int index, double value);
static double QCDoubleArrayGetAt(QCArrayRef x, int index);
static double QCDoubleArrayMax(QCArrayRef x);
static QCArrayRef QCDoubleArrayRealParts(QCArrayRef x);
static QCArrayRef QCDoubleArrayComplexMultiply(QCArrayRef x, QCArrayRef y);
static QCArrayRef QCDoubleArrayGetNoZeroIndices(QCArrayRef array);
static void QCDoubleArrayPrint(QCArrayRef array);
static bool QCDoubleArrayCompareRaw(QCArrayRef array, const void *expected, QCArrayDataType dataType);
static void QCDoubleArrayAppend(QCArrayRef array, QCArrayRef other);
static QCArrayRef QCDoubleArraySlice(QCArrayRef array, size_t start, size_t end);
static QCArrayRef QCDoubleArrayConvert(QCArrayRef array, QCArrayDataType type);

static struct QCArrayClass kQCDoubleArrayClass = {
//        .base = kQCArrayClassRef,
        .name = "QCDoubleArray",
        .allocator = QCAllocator,
        .size = sizeof(struct QCArray),
        .deallocate = QCArrayDeallocate,
        .copy = QCDoubleArrayCopy,
        .enumerator = QCDoubleArrayEnumerator,
        .add = QCDoubleArrayAdd,
        .multiply = QCDoubleArrayMultiply,
        .round = QCDoubleArrayRound,
        .mod = QCDoubleArrayMod,
        .zero = QCDoubleArrayZeroCount,
        .addAt = QCDoubleArrayAddAt,
        .xorAt = QCDoubleArrayXORAt,
        .set = QCDoubleArraySetAt,
        .get = QCDoubleArrayGetAt,
        .max = QCDoubleArrayMax,
        .real = QCDoubleArrayRealParts,
        .complexMultiply = QCDoubleArrayComplexMultiply,
        .nonzeroIndices = QCDoubleArrayGetNoZeroIndices,
        .print = QCDoubleArrayPrint,
        .compareRaw = QCDoubleArrayCompareRaw,
        .append = QCDoubleArrayAppend,
        .slice = QCDoubleArraySlice,
        .convert = QCDoubleArrayConvert
};

const QCClassRef kQCDoubleArrayClassRef = &kQCDoubleArrayClass;

const void *QCDoubleArrayCreate(const void *initData, size_t count, bool needCopy) {
    if (count > 0) {
        QCArrayDataType type = QCDTDouble;
        QCArrayRef array = QCAllocate(&kQCDoubleArrayClass);
        array->isa = kQCDoubleArrayClassRef;

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

static const void *QCDoubleArrayCopy(QCArrayRef array) {
    if (array) {
        return QCDoubleArrayCreate(array->data, array->count, true);
    }
    return NULL;
}

static void QCDoubleArrayEnumerator(QCArrayRef array, const void *func, const void *ctx) {
    QCFOREACH(array, ((QCDoubleLoopFunc)func)(d[i], i, ctx), double);
}

static QCArrayRef QCDoubleArrayAdd (QCArrayRef array, QCArrayRef y) {
    if (y->isa == kQCDoubleArrayClassRef) {
        // both double
        double *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], double);
    } else {
        int *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], double);
    }
    return array;
}

static QCArrayRef QCDoubleArrayMultiply (QCArrayRef array, double mul) {
    QCFOREACH(array, d[i] *= mul, double);
}
static QCArrayRef QCDoubleArrayRound (QCArrayRef array) {
    QCFOREACH(array, d[i] = round(d[i]), double);
}
static QCArrayRef QCDoubleArrayMod (QCArrayRef array, int mod) {
    QCFOREACH(array, d[i] = (int)d[i] % mod, double);
}

static size_t QCDoubleArrayZeroCount (QCArrayRef array) {
    size_t total = 0;
    QCFOREACH(array, if (d[i] == 0){ ++total; }, double);
    return total;
}

QCARRAYIMP(QCDoubleArray, double)

static void QCDoubleArrayPrint(QCArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x>[ ", array->isa->name, array);

        QCFOREACH(array, printf("%f, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, double);

        printf(" ]\n");
    }
}

static bool QCInt32ArrayEqual(QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTInt) {
        int *dy = y->data;
        QCFOREACH(array, if ((int)d[i] != dy[i]) { return false; }, double);
        return true;
    } else {
        return memcmp(array->data, y->data, array->count * sizeof(size_t)) == 0;
    }
}

static double QCDoubleArrayMax(QCArrayRef array) {
    double max = 0;
    QCFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, double);
    return max;
}

static QCArrayRef QCDoubleArrayRealParts(QCArrayRef x) {
    if (x) {
        int count = x->count;
        if (x->fft) {
            count -= 2;
        }
        double *array = x->data;
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

static QCArrayRef QCDoubleArrayComplexMultiply(QCArrayRef xArray, QCArrayRef yArray) {
    if (yArray->datatype == QCDTDouble) {
        // both double
        int count = xArray->count;
        double *x = xArray->data;
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
    } else {
        int count = xArray->count;
        double *x = xArray->data;
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
    }
}

static QCArrayRef QCDoubleArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        size_t count = array->count;
        double *x = array->data;
        int *indices = _QCMallocData(QCDTInt, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if ((int)x[i] != 0) {
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

QCARRAYCOMPARE(_arrayCompareInt, double, int, "not equal: %d %f %d\n")

QCARRAYCOMPARE(_arrayCompareByte, double , QCByte, "not equal: %d %f %d\n")

static bool QCDoubleArrayCompareRaw(QCArrayRef array, const void *expected, QCArrayDataType dataType) {
    switch (dataType) {
        case QCDTInt: {
            return _arrayCompareInt(array->data, expected, array->count);
        }
        case QCDTByte: {
            return _arrayCompareByte(array->data, expected, array->count);
        }
        default: {
            return memcmp(array->data, expected, sizeof(double) * array->count) == 0;
        }

    }
    return false;
}

// TODO
static QCArrayRef QCDoubleArrayConvert(QCArrayRef array, QCArrayDataType type) {
    if (array) {
        switch (type) {
            case QCDTInt: {
            }
            case QCDTDouble: {
                return QCDoubleArrayCreate(array->data, array->count, true);
            }
            case QCDTByte: {

            }
            default: {
                return NULL;
            }
        }
    }
    return NULL;
}
