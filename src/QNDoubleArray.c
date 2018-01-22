//
// Created by Isaac on 2018/1/12.
//

#include <memory.h>
#include <math.h>
#include "QNDoubleArray.h"

static const void *QNDoubleArrayCopy(QNArrayRef array);

static void QNDoubleArrayEnumerator(QNArrayRef array, const void *func, const void *ctx);
static QNArrayRef QNDoubleArrayAdd (QNArrayRef array, QNArrayRef y);
static QNArrayRef QNDoubleArrayMultiply (QNArrayRef array, double mul);
static QNArrayRef QNDoubleArrayRound (QNArrayRef array);
static QNArrayRef QNDoubleArrayMod (QNArrayRef array, int mod);
static size_t QNDoubleArrayZeroCount (QNArrayRef array);
static void QNDoubleArrayAddAt(QNArrayRef x, int index, double value);
static void QNDoubleArrayXORAt(QNArrayRef x, int index, int value);
static void QNDoubleArraySetAt(QNArrayRef x, int index, double value);
static double QNDoubleArrayGetAt(QNArrayRef x, int index);
static double QNDoubleArrayMax(QNArrayRef array);
static QNArrayRef QNDoubleArrayRealParts(QNArrayRef x);
static QNArrayRef QNDoubleArrayComplexMultiply(QNArrayRef , QNArrayRef );
static QNArrayRef QNDoubleArrayGetNoZeroIndices(QNArrayRef array);
static void QNDoubleArrayPrint(QNArrayRef array);
static bool QNDoubleArrayCompareRaw(QNArrayRef array, const void *expected, QNArrayDataType dataType);
static void QNDoubleArrayAppend(QNArrayRef array, QNArrayRef other);
static QNArrayRef QNDoubleArraySlice(QNArrayRef array, size_t start, size_t end);
static QNArrayRef QNDoubleArrayConvert(QNArrayRef array, QNArrayDataType type);
static QNArrayRef QNDoubleArrayPack(QNArrayRef array);
static bool QNDoubleArrayEqual(QNArrayRef array, QNArrayRef y);
static bool QNDoubleArraySaveFile(QNArrayRef array, FILE *fp);
static size_t QNDoubleArrayGetSize(QNArrayRef );

static struct QNArrayClass kQNDoubleArrayClass = {
//        .base = kQNArrayClassRef,
        .name = "QNDoubleArray",
        .allocator = QNAllocator,
        .size = sizeof(struct QNArray),
        .deallocate = QNArrayDeallocate,
        .copy = QNDoubleArrayCopy,
        .enumerator = QNDoubleArrayEnumerator,
        .add = QNDoubleArrayAdd,
        .multiply = QNDoubleArrayMultiply,
        .round = QNDoubleArrayRound,
        .mod = QNDoubleArrayMod,
        .zero = QNDoubleArrayZeroCount,
        .addAt = QNDoubleArrayAddAt,
        .xorAt = QNDoubleArrayXORAt,
        .set = QNDoubleArraySetAt,
        .get = QNDoubleArrayGetAt,
        .max = QNDoubleArrayMax,
        .real = QNDoubleArrayRealParts,
        .complexMultiply = QNDoubleArrayComplexMultiply,
        .nonzeroIndices = QNDoubleArrayGetNoZeroIndices,
        .print = QNDoubleArrayPrint,
        .compareRaw = QNDoubleArrayCompareRaw,
        .append = QNDoubleArrayAppend,
        .slice = QNDoubleArraySlice,
        .convert = QNDoubleArrayConvert,
        .pack = QNDoubleArrayPack,
        .equal = QNDoubleArrayEqual,
        .saveFile = QNDoubleArraySaveFile,
        .getSize = QNDoubleArrayGetSize
};

const QNClassRef kQNDoubleArrayClassRef = &kQNDoubleArrayClass;

QNArrayRef QNDoubleArrayCreate(const void *initData, size_t count, bool needCopy) {
    if (count > 0) {
        QNArrayDataType type = QNDTDouble;
        QNArrayRef array = QNAllocate(&kQNDoubleArrayClass);
        array->isa = kQNDoubleArrayClassRef;

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

QNArrayRef QNDoubleArrayFromFile(FILE *fp) {
    size_t count = 0;
    fread(&count, sizeof(size_t), 1, fp);
    double *data = QNAllocator(sizeof(double) * count);
    fread(data, sizeof(double), count, fp);
    QNArrayRef ref = QNArrayCreateWithDouble(data, count, false);
    ref->needfree = true;

    return ref;
}

static const void *QNDoubleArrayCopy(QNArrayRef array) {
    if (array) {
        return QNDoubleArrayCreate(array->data, array->count, true);
    }
    return NULL;
}

static void QNDoubleArrayEnumerator(QNArrayRef array, const void *func, const void *ctx) {
    QNFOREACH(array, ((QNDoubleLoopFunc)func)(d[i], i, ctx), double);
}

static QNArrayRef QNDoubleArrayAdd (QNArrayRef array, QNArrayRef y) {
    if (y->isa == kQNDoubleArrayClassRef) {
        // both double
        double *dy = y->data;
        QNFOREACH(array, d[i] += dy[i], double);
    } else {
        int *dy = y->data;
        QNFOREACH(array, d[i] += dy[i], double);
    }
    return array;
}

static QNArrayRef QNDoubleArrayMultiply (QNArrayRef array, double mul) {
    QNFOREACH(array, d[i] *= mul, double);
}
static QNArrayRef QNDoubleArrayRound (QNArrayRef array) {
    QNFOREACH(array, d[i] = round(d[i]), double);
}
static QNArrayRef QNDoubleArrayMod (QNArrayRef array, int mod) {
    QNFOREACH(array, d[i] = (int)d[i] % mod, double);
}

static size_t QNDoubleArrayZeroCount (QNArrayRef array) {
    size_t total = 0;
    QNFOREACH(array, if (d[i] == 0){ ++total; }, double);
    return total;
}

QNARRAYIMP(QNDoubleArray, double)

static void QNDoubleArrayPrint(QNArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x>[ ", array->isa->name, array);

        QNFOREACH(array, printf("%f, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, double);

        printf(" ]\n");
    }
}

static bool QNDoubleArrayEqual(QNArrayRef array, QNArrayRef y) {
    if (y->datatype == QNDTInt) {
        int *dy = y->data;
        QNFOREACH(array, if ((int)d[i] != dy[i]) { return false; }, double);
        return true;
    } else {
        return memcmp(array->data, y->data, array->count * sizeof(double)) == 0;
    }
}

static double QNDoubleArrayMax(QNArrayRef array) {
    double max = 0;
    QNFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, double);
    return max;
}

static QNArrayRef QNDoubleArrayRealParts(QNArrayRef x) {
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
        QNArrayRef ret = QNArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
    }
    return NULL;
}

static QNArrayRef QNDoubleArrayComplexMultiply(QNArrayRef xArray, QNArrayRef yArray) {
    if (yArray->datatype == QNDTDouble) {
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
        QNArrayRef ret = QNArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
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
        QNArrayRef ret = QNArrayCreateWithDouble(out, count, false);
        ret->needfree = true;
        return ret;
    }
}

static QNArrayRef QNDoubleArrayGetNoZeroIndices(QNArrayRef array) {
    if (array) {
        size_t count = array->count;
        double *x = array->data;
        int *indices = _QNMallocData(QNDTInt, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if ((int)x[i] != 0) {
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

QNARRAYCOMPARE(_arrayCompareInt, double, int, "not equal: %d %f %d\n")

QNARRAYCOMPARE(_arrayCompareByte, double , QNByte, "not equal: %d %f %d\n")

QNARRAYCOMPARE(_arrayCompareDouble, double , double , "not equal: %d %f %f\n")

static bool QNDoubleArrayCompareRaw(QNArrayRef array, const void *expected, QNArrayDataType dataType) {
    switch (dataType) {
        case QNDTInt: {
            return _arrayCompareInt(array->data, expected, array->count);
        }
        case QNDTByte: {
            return _arrayCompareByte(array->data, expected, array->count);
        }
        default: {
            return _arrayCompareDouble(array->data, expected, array->count);
        }

    }
    return false;
}

// TODO
static QNArrayRef QNDoubleArrayConvert(QNArrayRef array, QNArrayDataType type) {
    if (array) {
        switch (type) {
            case QNDTInt: {
            }
            case QNDTDouble: {
                return QNDoubleArrayCreate(array->data, array->count, true);
            }
            case QNDTByte: {

            }
            default: {
                return NULL;
            }
        }
    }
    return NULL;
}
