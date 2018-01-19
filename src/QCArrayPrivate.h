//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAYPRIVATE_H
#define PQC_CRYPTO_QCARRAYPRIVATE_H


#include "QCArray.h"
#include "QCClass.h"
#include "QCObject.h"
#include <openssl/bn.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (* QCArrayEnumeratorFunc) (QCArrayRef array, const void *func, const void *ctx);
typedef QCArrayRef (* QCArrayAddFunc) (QCArrayRef x, QCArrayRef y);
typedef QCArrayRef (* QCArrayMultiplyFunc) (QCArrayRef x, double mul);
typedef QCArrayRef (* QCArrayRoundFunc) (QCArrayRef x);
typedef QCArrayRef (* QCArrayModFunc) (QCArrayRef x, int mod);
typedef size_t (* QCArrayZeroCountFunc) (QCArrayRef x);
typedef void (* QCArrayAddAtFunc)(QCArrayRef x, int index, double value);
typedef void (* QCArrayXORAtFunc)(QCArrayRef x, int index, int value);
typedef void (* QCArraySetAtFunc)(QCArrayRef x, int index, double value);
typedef double (* QCArrayGetAtFunc)(QCArrayRef x, int index);
typedef double (* QCArrayMaxFunc)(QCArrayRef x);
typedef QCArrayRef (* QCArrayRealPartsFunc)(QCArrayRef x);
typedef QCArrayRef (* QCArrayComplexMultiplyFunc)(QCArrayRef x, QCArrayRef y);
typedef QCArrayRef (* QCArrayNonZeroIndicesFunc)(QCArrayRef array);
typedef QCArrayRef (* QCArrayHashFunc)(QCArrayRef array);
typedef bool (* QCArrayCompareRawFunc)(QCArrayRef array, const void *, QCArrayDataType);
typedef void (* QCArrayAppendFunc)(QCArrayRef array, QCArrayRef other);
typedef QCArrayRef (* QCArraySliceFunc)(QCArrayRef array, size_t start, size_t end);
typedef QCArrayRef (* QCArrayConvertFunc)(QCArrayRef array, QCArrayDataType type);
typedef QCArrayRef (* QCArrayPackFunc)(QCArrayRef array);

struct QCArrayClass {
    QCCLASSFIELDS
    QCArrayEnumeratorFunc enumerator;
    QCArrayAddFunc add;
    QCArrayMultiplyFunc multiply;
    QCArrayRoundFunc round;
    QCArrayModFunc mod;
    QCArrayZeroCountFunc zero;
    QCArrayAddAtFunc addAt;
    QCArrayXORAtFunc xorAt;
    QCArraySetAtFunc set;
    QCArrayGetAtFunc get;
    QCArrayMaxFunc max;
    QCArrayRealPartsFunc real;
    QCArrayComplexMultiplyFunc complexMultiply;
    QCArrayNonZeroIndicesFunc nonzeroIndices;
    QCArrayHashFunc sha256;
    QCArrayHashFunc sha512;
    QCArrayCompareRawFunc compareRaw;
    QCArrayAppendFunc append;
    QCArrayConvertFunc convert;
    QCArraySliceFunc slice;
    QCArrayPackFunc pack;
};

typedef struct QCArrayClass *QCArrayClassRef;

struct QCArray {
    QCOBJECTFIELDS
    void *data;
    size_t count; // count of number in data
    struct {
        unsigned int fft: 1; // data contains fft result
        unsigned int needfree: 1; // if data need to be freed
        unsigned int datatype: 3; // data type
        unsigned int unused: 27;
    };
};

extern void *_QCMallocData(QCArrayDataType type, size_t count, size_t  *size);

/*
 * exp poly
 */
extern QCArrayRef QCArrayExpPoly(QCArrayRef array, BIGNUM *n);

#ifndef QCARRAYIMP

#define QCARRAYIMP(CLASS, TYPE) static void CLASS##AddAt(QCArrayRef x, int index, double value) { \
    TYPE *d = x->data; \
    d[index] += (TYPE)value; \
} \
static void CLASS ## XORAt(QCArrayRef x, int index, int value) { \
    TYPE *d = x->data; \
    d[index] = (int)d[index] ^ value; \
} \
static void CLASS ## SetAt(QCArrayRef x, int index, double value) { \
    TYPE *d = x->data; \
    d[index] = (TYPE)value; \
} \
static double CLASS ## GetAt(QCArrayRef x, int index) { \
    TYPE *d = x->data; \
    return d[index]; \
} \
static void CLASS ## Append(QCArrayRef array, QCArrayRef other) { \
    if (array && other) { \
        QCArrayRef co = QCArrayConvert(other, array->datatype); \
        if (co) { \
            size_t size = sizeof(TYPE) * array->count; \
            size_t otherSize = sizeof(TYPE) * co->count; \
            TYPE * p = array->isa->allocator(size + otherSize); \
            memcpy(p, array->data, size); \
            memcpy(p + size, other->data, otherSize); \
            if (array->needfree) { \
                QCDeallocate(array->data); \
            } \
            array->needfree = true; \
            array->data = p; \
            array->count += co->count; \
            if (co != other) { QCRelease(co); } \
        } \
    } \
} \
static QCArrayRef CLASS ## Slice(QCArrayRef array, size_t start, size_t end) { \
    const size_t count = array->count; \
    TYPE *data = array->data; \
    if (start >= 0 && end >= 0 && start < count && end <= count && start < end) { \
        return CLASS ## Create(data + start, end - start, true); \
    } \
    return NULL; \
} \
static QCArrayRef CLASS ## Pack(QCArrayRef array) { \
    size_t count = array->count; \
    QCByte *str = array->isa->allocator(count * sizeof(QCByte)); \
    TYPE *data = array->data; \
    for (size_t i = 0; i < count; ++i) { \
        sprintf(str + i, "%d", (int)data[i]); \
    } \
    QCArrayRef a1 = QCArrayCreateWithByte(str, count, false); \
    a1->needfree = true; \
    QCArrayRef result = QCArraySHA512(a1); \
    QCRelease(a1); \
    return result; \
}

#endif

#ifndef QCFOREACH

#define QCFOREACH(array, exp, TYPE) do { \
                        TYPE *d = array->data; \
                        for (size_t i = 0; i < array->count; ++i) { exp; } \
                    } while(0)

#endif

#ifndef QCARRAYCOMPARE

#define QCARRAYCOMPARE(NAME, T1, T2, FMT) static bool NAME(const T1 *array, const T2 *expected, size_t count) { \
    bool equal = true; \
    if (array && expected) { \
        int total = 0; \
        for (size_t i = 0; i < count; ++i) { \
            if (fabs(array[i] - expected[i]) > 0.00000005) { \
                printf(FMT, i, array[i], expected[i]); \
                equal = false; \
                ++total; \
            } \
        } \
        if (!equal) { \
            printf("total not equal: %d rate: %.2f%%", total, total * 100.0 / count); \
        } \
    } \
    return equal; \
}

#endif

extern QC_STRONG const char *QCEncodeBase64(QCByte *data, size_t length);

/*
 * free array's data
 */
extern void QCArrayDeallocate(QCArrayRef array);

#endif //PQC_CRYPTO_QCARRAYPRIVATE_H
