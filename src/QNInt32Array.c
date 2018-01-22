//
// Created by Isaac on 2018/1/12.
//

#include "QNInt32Array.h"
#include <memory.h>
#include <math.h>
#include <fftw3.h>

static void QNInt32ArrayEnumerator(QNArrayRef array, const void *func, const void *ctx);
static const void *QNInt32ArrayCopy(QNArrayRef array);
static QNArrayRef QNInt32ArrayAdd (QNArrayRef , QNArrayRef );
static QNArrayRef QNInt32ArrayMultiply (QNArrayRef , double );
static QNArrayRef QNInt32ArrayRound (QNArrayRef );
static QNArrayRef QNInt32ArrayMod (QNArrayRef , int );
static size_t QNInt32ArrayZeroCount (QNArrayRef );
static void QNInt32ArrayAddAt(QNArrayRef x, int index, double value);
static void QNInt32ArrayXORAt(QNArrayRef x, int index, int value);
static void QNInt32ArraySetAt(QNArrayRef x, int index, double value);
static double QNInt32ArrayGetAt(QNArrayRef x, int index);
static void QNInt32ArrayPrint(QNArrayRef );
static bool QNInt32ArrayEqual(QNArrayRef , QNArrayRef );
static double QNInt32ArrayMax(QNArrayRef array);
static QNArrayRef QNInt32ArraySlice(QNArrayRef array, size_t start, size_t end);
static QNArrayRef QNInt32ArrayRealParts(QNArrayRef x);
static QNArrayRef QNInt32ArrayComplexMultiply(QNArrayRef xArray, QNArrayRef yArray);
static QNArrayRef QNInt32ArrayGetNoZeroIndices(QNArrayRef array);
static QNArrayRef QNInt32ArraySHA256(QNArrayRef array);
static bool QNInt32ArrayCompareRaw(QNArrayRef array, const void *expected, QNArrayDataType dataType);
static QNArrayRef QNInt32ArrayPack(QNArrayRef array);
static bool QNInt32ArraySaveFile(QNArrayRef array, FILE *fp);
static size_t QNInt32ArrayGetSize(QNArrayRef );

static struct QNArrayClass kQNInt32ArrayClass = {
//        .base = kQNArrayClassRef,
        .name = "QNInt32Array",
        .allocator = QNAllocator,
        .size = sizeof(struct QNArray),
        .deallocate = QNArrayDeallocate,
        .copy = QNInt32ArrayCopy,
        .enumerator = QNInt32ArrayEnumerator,
        .add = QNInt32ArrayAdd,
        .multiply = QNInt32ArrayMultiply,
        .round = QNInt32ArrayRound,
        .mod = QNInt32ArrayMod,
        .zero = QNInt32ArrayZeroCount,
        .addAt = QNInt32ArrayAddAt,
        .xorAt = QNInt32ArrayXORAt,
        .set = QNInt32ArraySetAt,
        .get = QNInt32ArrayGetAt,
        .print = QNInt32ArrayPrint,
        .equal = QNInt32ArrayEqual,
        .max = QNInt32ArrayMax,
        .real = QNInt32ArrayRealParts,
        .complexMultiply = QNInt32ArrayComplexMultiply,
        .nonzeroIndices = QNInt32ArrayGetNoZeroIndices,
        .slice = QNInt32ArraySlice,
        .sha256 = QNInt32ArraySHA256,
        .compareRaw = QNInt32ArrayCompareRaw,
        .pack = QNInt32ArrayPack,
        .saveFile = QNInt32ArraySaveFile,
        .getSize = QNInt32ArrayGetSize
};

const QNClassRef kQNInt32ArrayClassRef = &kQNInt32ArrayClass;

const QNArrayRef QNInt32ArrayCreate(const void *initData, size_t count, bool needCopy) {
    if (count > 0) {
        QNArrayDataType type = QNDTInt;
        QNArrayRef array = QNAllocate(&kQNInt32ArrayClass);
        array->isa = kQNInt32ArrayClassRef;

        if (initData) {
            if (needCopy) {
                size_t size = 0;
                array->data = _QNMallocData(type, count, &size);
                memcpy(array->data, initData, size);
            } else {
                array->data = initData;
            }
            array->needfree = needCopy;
        } else {
            array->data = _QNMallocData(type, count, NULL);
            array->needfree = true;
        }
        array->count = count;
        array->fft = false;
        array->datatype = type;
        return array;
    }
    return NULL;
}

static const void *QNInt32ArrayCopy(QNArrayRef array) {
    if (array) {
        return QNInt32ArrayCreate(array->data, array->count, true);
    }
    return NULL;
}

QNArrayRef QNInt32ArrayFromFile(FILE *fp) {
    size_t count = 0;
    fread(&count, sizeof(size_t), 1, fp);
    int *data = QNAllocator(sizeof(int) * count);
    fread(data, sizeof(int), count, fp);
    QNArrayRef ref = QNArrayCreateWithInt(data, count, false);
    ref->needfree = true;

    return ref;
}

static void QNInt32ArrayEnumerator(QNArrayRef array, const void *func, const void *ctx) {
    QNInt32LoopFunc f = func;
    QNFOREACH(array, f(d[i], i, ctx), int);
}

static QNArrayRef QNInt32ArrayAdd (QNArrayRef array, QNArrayRef y) {
    if (y->isa == kQNInt32ArrayClassRef) {
        // both int
        int *dy = y->data;
        QNFOREACH(array, d[i] += dy[i], int);
    } else {
        double *dy = y->data;
        QNFOREACH(array, d[i] += dy[i], int);
    }
    return array;
}

static QNArrayRef QNInt32ArrayMultiply (QNArrayRef array, double mul) {
    int m = (int)mul;
    QNFOREACH(array, d[i] *= m, int);
}
static QNArrayRef QNInt32ArrayRound (QNArrayRef array) {
    // no need to round
    return array;
}
static QNArrayRef QNInt32ArrayMod (QNArrayRef array, int mod) {
    QNFOREACH(array, d[i] = d[i] % mod, int);
}

static size_t QNInt32ArrayZeroCount (QNArrayRef array) {
    size_t total = 0;
    QNFOREACH(array, if (d[i] == 0){ ++total; }, int);
    return total;
}

QNARRAYIMP(QNInt32Array, int)

static void QNInt32ArrayPrint(QNArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x>[ ", array->isa->name, array);

        QNFOREACH(array, printf("%d, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, int);

        printf(" ]\n");
    }
}

static bool QNInt32ArrayEqual(QNArrayRef array, QNArrayRef y) {
    if (y->datatype == QNDTInt) {
        return memcmp(array->data, y->data, array->count * sizeof(int)) == 0;
    } else {
        double *dy = y->data;
        QNFOREACH(array, if (d[i] != (int)dy[i]) { return false; }, int);
        return true;
    }
}

static double QNInt32ArrayMax(QNArrayRef array) {
    int max = 0;
    QNFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, int);
    return max;
}

static QNArrayRef QNInt32ArrayRealParts(QNArrayRef x) {
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
        QNArrayRef ret = QNArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
    }
    return NULL;
}


static QNArrayRef QNInt32ArrayComplexMultiply(QNArrayRef xArray, QNArrayRef yArray) {
    if (yArray->datatype == QNDTInt) {
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
        QNArrayRef ret = QNArrayCreateWithDouble(out, count, false);
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
        QNArrayRef ret = QNArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
    }
}

static QNArrayRef QNInt32ArrayGetNoZeroIndices(QNArrayRef array) {
    if (array) {
        size_t count = array->count;
        int *x = array->data;
        int *indices = _QNMallocData(QNDTInt, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if (x[i] != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        QNArrayRef ref = QNArrayCreateWithInt(indices, idx, false);
        ref->needfree = true;
        return ref;
    }
    return NULL;
}


QNARRAYCOMPARE(_arrayCompareDouble, int, double, "not equal: %d %d %f\n")

QNARRAYCOMPARE(_arrayCompareByte, int, QNByte, "not equal: %d %d %d\n")

static bool QNInt32ArrayCompareRaw(QNArrayRef array, const void *expected, QNArrayDataType dataType) {
    switch (dataType) {
        case QNDTDouble: {
            return _arrayCompareDouble(array->data, expected, array->count);
        }
        case QNDTByte: {
            return _arrayCompareByte(array->data, expected, array->count);
        }
        default: {
            return memcmp(array->data, expected, sizeof(int) * array->count) == 0;
        }

    }
    return false;
}

static QNByte *_int32ArrayToCharArray(int *array, size_t count) {

}

static QNArrayRef QNInt32ArraySHA256(QNArrayRef array) {
    if (array) {
        QNArrayRef ba = QNArrayConvert(array, QNDTByte);
        QNArrayRef result = QNArraySHA256(ba);
        QNRelease(ba);
        return result;
    }
    return NULL;
}

static void intToByteArray(int value, QNByte out[4]) {
    out[0] = (QNByte)((value >> 24) & 0xFF);
    out[0] = (QNByte)((value >> 16) & 0xFF);
    out[0] = (QNByte)((value >> 8) & 0xFF);
    out[0] = (QNByte)(value & 0xFF);
}

static QNArrayRef QNInt32ArrayConvert(QNArrayRef array, QNArrayDataType type) {
    if (array) {
        switch (type) {
            case QNDTInt: {
                return QNInt32ArrayCreate(array->data, array->count, true);
            }
            case QNDTDouble: {

            }
            case QNDTByte: {
                size_t count = array->count;
                int *data = array->data;
                QNByte *d = array->isa->allocator(sizeof(QNByte) * 4 * count);
                for (size_t i = 0; i < count; ++i) {
                    intToByteArray(data[i], d + i * 4);
                }
                QNArrayRef result = QNArrayCreateWithByte(d, count * 4, false);
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
