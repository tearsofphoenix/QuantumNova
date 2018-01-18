//
// Created by Isaac on 2018/1/12.
//

#include "QCInt32Array.h"
#include <memory.h>
#include <math.h>
#include <fftw3.h>

static void QCInt32ArrayEnumerator(QCArrayRef array, const void *func, const void *ctx);
static const void *QCInt32ArrayCopy(QCArrayRef array);
static QCArrayRef QCInt32ArrayAdd (QCArrayRef , QCArrayRef );
static QCArrayRef QCInt32ArrayMultiply (QCArrayRef , double );
static QCArrayRef QCInt32ArrayRound (QCArrayRef );
static QCArrayRef QCInt32ArrayMod (QCArrayRef , int );
static size_t QCInt32ArrayZeroCount (QCArrayRef );
static void QCInt32ArrayAddAt(QCArrayRef x, int index, double value);
static void QCInt32ArrayXORAt(QCArrayRef x, int index, int value);
static void QCInt32ArraySetAt(QCArrayRef x, int index, double value);
static double QCInt32ArrayGetAt(QCArrayRef x, int index);
static void QCInt32ArrayPrint(QCArrayRef );
static bool QCInt32ArrayEqual(QCArrayRef , QCArrayRef );
static double QCInt32ArrayMax(QCArrayRef array);
static QCArrayRef QCInt32ArraySlice(QCArrayRef array, size_t start, size_t end);
static QCArrayRef QCInt32ArrayRealParts(QCArrayRef x);
static QCArrayRef QCInt32ArrayComplexMultiply(QCArrayRef xArray, QCArrayRef yArray);
static QCArrayRef QCInt32ArrayGetNoZeroIndices(QCArrayRef array);
static QCArrayRef QCInt32ArraySHA256(QCArrayRef array);
static bool QCInt32ArrayCompareRaw(QCArrayRef array, const void *expected, QCArrayDataType dataType);
static QCArrayRef QCInt32ArrayPack(QCArrayRef array);

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
        .slice = QCInt32ArraySlice,
        .sha256 = QCInt32ArraySHA256,
        .compareRaw = QCInt32ArrayCompareRaw,
        .pack = QCInt32ArrayPack
};

const QCClassRef kQCInt32ArrayClassRef = &kQCInt32ArrayClass;

const QCArrayRef QCInt32ArrayCreate(const void *initData, size_t count, bool needCopy) {
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

        QCFOREACH(array, printf("%d, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, int);

        printf(" ]\n");
    }
}

static bool QCInt32ArrayEqual(QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTInt) {
        return memcmp(array->data, y->data, array->count * sizeof(int)) == 0;
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
        QCArrayRef ret = QCArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
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
        QCArrayRef ret = QCArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
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
        QCArrayRef ret = QCArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
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

        QCArrayRef ref = QCArrayCreateWithInt(indices, idx, false);
        ref->needfree = true;
        return ref;
    }
    return NULL;
}


QCARRAYCOMPARE(_arrayCompareDouble, int, double, "not equal: %d %d %f\n")

QCARRAYCOMPARE(_arrayCompareByte, int, QCByte, "not equal: %d %d %d\n")

static bool QCInt32ArrayCompareRaw(QCArrayRef array, const void *expected, QCArrayDataType dataType) {
    switch (dataType) {
        case QCDTDouble: {
            return _arrayCompareDouble(array->data, expected, array->count);
        }
        case QCDTByte: {
            return _arrayCompareByte(array->data, expected, array->count);
        }
        default: {
            return memcmp(array->data, expected, sizeof(int) * array->count) == 0;
        }

    }
    return false;
}

static QCByte *_int32ArrayToCharArray(int *array, size_t count) {

}

static QCArrayRef QCInt32ArraySHA256(QCArrayRef array) {
    if (array) {
        QCArrayRef ba = QCArrayConvert(array, QCDTByte);
        QCArrayRef result = QCArraySHA256(ba);
        QCRelease(ba);
        return result;
    }
    return NULL;
}

static void intToByteArray(int value, QCByte out[4]) {
    out[0] = (QCByte)((value >> 24) & 0xFF);
    out[0] = (QCByte)((value >> 16) & 0xFF);
    out[0] = (QCByte)((value >> 8) & 0xFF);
    out[0] = (QCByte)(value & 0xFF);
}

static QCArrayRef QCInt32ArrayConvert(QCArrayRef array, QCArrayDataType type) {
    if (array) {
        switch (type) {
            case QCDTInt: {
                return QCInt32ArrayCreate(array->data, array->count, true);
            }
            case QCDTDouble: {

            }
            case QCDTByte: {
                size_t count = array->count;
                int *data = array->data;
                QCByte *d = array->isa->allocator(sizeof(QCByte) * 4 * count);
                for (size_t i = 0; i < count; ++i) {
                    intToByteArray(data[i], d + i * 4);
                }
                QCArrayRef result = QCArrayCreateWithByte(d, count * 4, false);
                result->needfree = true;
                return result;
            }
            default: {
                return NULL;
            }
        }
    }
    return NULL;
}
