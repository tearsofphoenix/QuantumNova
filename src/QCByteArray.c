//
// Created by Isaac on 2018/1/13.
//

#include "QCByteArray.h"
#include "vendor/sha256.h"
#include <memory.h>
#include <printf.h>
#include <fftw3.h>

static void QCByteArrayEnumerator(QCArrayRef array, const void *func, const void *ctx);
static const void *QCByteArrayCopy(QCArrayRef array);
static QCArrayRef QCByteArrayAdd (QCArrayRef x, QCArrayRef y);
static QCArrayRef QCByteArrayMultiply (QCArrayRef x, double mul);
static QCArrayRef QCByteArrayRound (QCArrayRef x);
static QCArrayRef QCByteArrayMod (QCArrayRef x, int mod);
static size_t QCByteArrayZeroCount (QCArrayRef x);
static void QCByteArrayAddAt(QCArrayRef x, int index, double value);
static void QCByteArrayXORAt(QCArrayRef x, int index, int value);
static void QCByteArraySetAt(QCArrayRef x, int index, double value);
static double QCByteArrayGetAt(QCArrayRef x, int index);
static void QCByteArrayPrint(QCArrayRef x);
static bool QCByteArrayEqual(QCArrayRef x, QCArrayRef y);
static double QCByteArrayMax(QCArrayRef array);

static QCArrayRef QCByteArrayGetNoZeroIndices(QCArrayRef array);
static QCArrayRef QCByteArraySHA256(QCArrayRef array);

static struct QCArrayClass kQCByteArrayClass = {
//        .base = kQCArrayClassRef,
        .name = "QCByteArray",
        .allocator = QCAllocator,
        .size = sizeof(struct QCArray),
        .deallocate = QCArrayDeallocate,
        .copy = QCByteArrayCopy,
        .enumerator = QCByteArrayEnumerator,
        .add = QCByteArrayAdd,
        .multiply = QCByteArrayMultiply,
        .round = QCByteArrayRound,
        .mod = QCByteArrayMod,
        .zero = QCByteArrayZeroCount,
        .addAt = QCByteArrayAddAt,
        .xorAt = QCByteArrayXORAt,
        .set = QCByteArraySetAt,
        .get = QCByteArrayGetAt,
        .print = QCByteArrayPrint,
        .equal = QCByteArrayEqual,
        .max = QCByteArrayMax,
        .nonzeroIndices = QCByteArrayGetNoZeroIndices,
        .sha256 = QCByteArraySHA256
};

const QCClassRef kQCByteArrayClassRef = &kQCByteArrayClass;

const void *QCByteArrayCreate(const void *initData, size_t count, bool needCopy) {
    if (count > 0) {
        QCArrayDataType type = QCDTByte;
        QCArrayRef array = QCAllocate(&kQCByteArrayClass);
        array->isa = kQCByteArrayClassRef;

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

static const void *QCByteArrayCopy(QCArrayRef array) {
    if (array) {
        return QCByteArrayCreate(array->data, array->count, true);
    }
    return NULL;
}


static void QCByteArrayEnumerator(QCArrayRef array, const void *func, const void *ctx) {
    QCByteArrayLoopFunc f = func;
    QCFOREACH(array, f(d[i], i, ctx), QCByte);
}

static QCArrayRef QCByteArrayAdd (QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTByte) {
        // both int
        QCByte *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], QCByte);
    } else {
        double *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], QCByte);
    }
    return array;
}

static QCArrayRef QCByteArrayMultiply (QCArrayRef array, double mul) {
    QCByte m = (QCByte)mul;
    QCFOREACH(array, d[i] *= m, QCByte);
}
static QCArrayRef QCByteArrayRound (QCArrayRef array) {
    // no need to round
    return array;
}
static QCArrayRef QCByteArrayMod (QCArrayRef array, int mod) {
    QCFOREACH(array, d[i] = (QCByte)(d[i] % mod), QCByte);
}

static size_t QCByteArrayZeroCount (QCArrayRef array) {
    size_t total = 0;
    QCFOREACH(array, if (d[i] == 0){ ++total; }, QCByte);
    return total;
}

QCARRAYIMP(QCByteArray, QCByte)

static void QCByteArrayPrint(QCArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x>[ ", array->isa->name, array);

        array->isa->print(array);
        QCFOREACH(array, printf("%d, ", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, QCByte);

        printf(" ]\n");
    }
}

static bool QCByteArrayEqual(QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTByte) {
        return memcmp(array->data, y->data, array->count * sizeof(size_t)) == 0;
    } else {
        double *dy = y->data;
        QCFOREACH(array, if (d[i] != (int)dy[i]) { return false; }, QCByte);
        return true;
    }
}

static double QCByteArrayMax(QCArrayRef array) {
    int max = 0;
    QCFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, QCByte);
    return max;
}

static QCArrayRef QCByteArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        size_t count = array->count;
        QCByte *x = array->data;
        QCByte *indices = _QCMallocData(QCDTByte, count, NULL);
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


static void QCSHA256(const void *data, size_t size, BYTE *buf) {
    SHA256_CTX ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, data, size);
    sha256_final(&ctx, buf);
}

static QCArrayRef QCByteArraySHA256(QCArrayRef array) {
    if (array) {
        QCByte *buf = array->isa->allocator(sizeof(QCByte) * SHA256_BLOCK_SIZE);
        QCSHA256(array->data, sizeof(QCByte) * array->count, buf);
        return QCByteArrayCreate(buf, SHA256_BLOCK_SIZE, false);
    }
    return NULL;
}