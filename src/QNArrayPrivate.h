//
// Created by Isaac on 2018/1/11.
//

#ifndef QN_QNARRAYPRIVATE_H
#define QN_QNARRAYPRIVATE_H


#include "QNArray.h"
#include "QNClass.h"
#include "QNObject.h"
#include <openssl/bn.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (* QNArrayEnumeratorFunc) (QNArrayRef array, const void *func, const void *ctx);
typedef QNArrayRef (* QNArrayAddFunc) (QNArrayRef x, QNArrayRef y);
typedef QNArrayRef (* QNArrayMultiplyFunc) (QNArrayRef x, double mul);
typedef QNArrayRef (* QNArrayRoundFunc) (QNArrayRef x);
typedef QNArrayRef (* QNArrayModFunc) (QNArrayRef x, int mod);
typedef size_t (* QNArrayZeroCountFunc) (QNArrayRef x);
typedef void (* QNArrayAddAtFunc)(QNArrayRef x, int index, double value);
typedef void (* QNArrayXORAtFunc)(QNArrayRef x, int index, int value);
typedef void (* QNArraySetAtFunc)(QNArrayRef x, int index, double value);
typedef double (* QNArrayGetAtFunc)(QNArrayRef x, int index);
typedef double (* QNArrayMaxFunc)(QNArrayRef x);
typedef QNArrayRef (* QNArrayRealPartsFunc)(QNArrayRef x);
typedef QNArrayRef (* QNArrayComplexMultiplyFunc)(QNArrayRef x, QNArrayRef y);
typedef QNArrayRef (* QNArrayNonZeroIndicesFunc)(QNArrayRef array);
typedef QNArrayRef (* QNArrayHashFunc)(QNArrayRef array);
typedef bool (* QNArrayCompareRawFunc)(QNArrayRef array, const void *, QNArrayDataType);
typedef void (* QNArrayAppendFunc)(QNArrayRef array, QNArrayRef other);
typedef QNArrayRef (* QNArraySliceFunc)(QNArrayRef array, size_t start, size_t end);
typedef QNArrayRef (* QNArrayConvertFunc)(QNArrayRef array, QNArrayDataType type);
typedef QNArrayRef (* QNArrayPackFunc)(QNArrayRef array);
typedef bool (* QNArraySaveFileFunc)(QNArrayRef array, FILE *fp);
typedef size_t (* QNArrayGetSizeFunc)(QNArrayRef array);

struct QNArrayClass {
    QNCLASSFIELDS
    QNArrayEnumeratorFunc enumerator;
    QNArrayAddFunc add;
    QNArrayMultiplyFunc multiply;
    QNArrayRoundFunc round;
    QNArrayModFunc mod;
    QNArrayZeroCountFunc zero;
    QNArrayAddAtFunc addAt;
    QNArrayXORAtFunc xorAt;
    QNArraySetAtFunc set;
    QNArrayGetAtFunc get;
    QNArrayMaxFunc max;
    QNArrayRealPartsFunc real;
    QNArrayComplexMultiplyFunc complexMultiply;
    QNArrayNonZeroIndicesFunc nonzeroIndices;
    QNArrayHashFunc sha256;
    QNArrayHashFunc sha512;
    QNArrayCompareRawFunc compareRaw;
    QNArrayAppendFunc append;
    QNArrayConvertFunc convert;
    QNArraySliceFunc slice;
    QNArrayPackFunc pack;
    QNArraySaveFileFunc saveFile;
    QNArrayGetSizeFunc getSize;
};

typedef struct QNArrayClass *QNArrayClassRef;

struct QNArray {
    QNOBJECTFIELDS
    void *data;
    size_t count; // count of number in data
    struct {
        unsigned int fft: 1; // data contains fft result
        unsigned int needfree: 1; // if data need to be freed
        unsigned int datatype: 3; // data type
        unsigned int unused: 27;
    };
};

extern void *_QNMallocData(QNArrayDataType type, size_t count, size_t  *size);

/*
 * exp poly
 */
extern QNArrayRef QNArrayExpPoly(QNArrayRef array, BIGNUM *n);

#ifndef QNARRAYIMP

#define QNARRAYIMP(CLASS, TYPE) static void CLASS##AddAt(QNArrayRef x, int index, double value) { \
    TYPE *d = x->data; \
    d[index] += (TYPE)value; \
} \
static void CLASS ## XORAt(QNArrayRef x, int index, int value) { \
    TYPE *d = x->data; \
    d[index] = (int)d[index] ^ value; \
} \
static void CLASS ## SetAt(QNArrayRef x, int index, double value) { \
    TYPE *d = x->data; \
    d[index] = (TYPE)value; \
} \
static double CLASS ## GetAt(QNArrayRef x, int index) { \
    TYPE *d = x->data; \
    return d[index]; \
} \
static void CLASS ## Append(QNArrayRef array, QNArrayRef other) { \
    if (array && other) { \
        QNArrayRef co = QNArrayConvert(other, array->datatype); \
        if (co) { \
            size_t size = sizeof(TYPE) * array->count; \
            size_t otherSize = sizeof(TYPE) * co->count; \
            TYPE * p = array->isa->allocator(size + otherSize); \
            memcpy(p, array->data, size); \
            memcpy(p + size, other->data, otherSize); \
            if (array->needfree) { \
                QNDeallocate(array->data); \
            } \
            array->needfree = true; \
            array->data = p; \
            array->count += co->count; \
            if (co != other) { QNRelease(co); } \
        } \
    } \
} \
static QNArrayRef CLASS ## Slice(QNArrayRef array, size_t start, size_t end) { \
    const size_t count = array->count; \
    TYPE *data = array->data; \
    if (start >= 0 && end >= 0 && start < count && end <= count && start < end) { \
        return CLASS ## Create(data + start, end - start, true); \
    } \
    return NULL; \
} \
static QNArrayRef CLASS ## Pack(QNArrayRef array) { \
    size_t count = array->count; \
    QNByte *str = array->isa->allocator(count * sizeof(QNByte)); \
    TYPE *data = array->data; \
    for (size_t i = 0; i < count; ++i) { \
        sprintf(str + i, "%d", (int)data[i]); \
    } \
    QNArrayRef a1 = QNArrayCreateWithByte(str, count, false); \
    a1->needfree = true; \
    QNArrayRef result = QNArraySHA512(a1); \
    QNRelease(a1); \
    return result; \
} \
static bool CLASS ## SaveFile(QNArrayRef array, FILE *fp) { \
    QNArrayDataType type = array->datatype; \
    fwrite(&type, sizeof(QNArrayDataType), 1, fp); \
    fwrite(&array->count, sizeof(size_t), 1, fp); \
    fwrite(array->data, sizeof(TYPE), array->count, fp); \
    return true; \
} \
static size_t CLASS ## GetSize(QNArrayRef array) { \
    return array->count * sizeof(TYPE); \
} \

#endif

#ifndef QNFOREACH

#define QNFOREACH(array, exp, TYPE) do { \
                        TYPE *d = array->data; \
                        for (size_t i = 0; i < array->count; ++i) { exp; } \
                    } while(0)

#endif

#ifndef QNARRAYCOMPARE

#define QNARRAYCOMPARE(NAME, T1, T2, FMT) static bool NAME(const T1 *array, const T2 *expected, size_t count) { \
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

extern QN_STRONG const char *QNEncodeBase64(QNByte *data, size_t length);

/*
 * free array's data
 */
extern void QNArrayDeallocate(QNArrayRef array);

#endif //QN_QNARRAYPRIVATE_H
