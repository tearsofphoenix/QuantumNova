//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAYPRIVATE_H
#define PQC_CRYPTO_QCARRAYPRIVATE_H


#include "QCArray.h"
#include "QCClass.h"
#include "QCObject.h"
#include <stdbool.h>
#include <ntsid.h>

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
typedef QCArrayRef (* QCArraySHA256Func)(QCArrayRef array);

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
    QCArraySHA256Func sha256;
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

extern const QCClassRef kQCArrayClassRef;

extern void *_QCMallocData(QCArrayDataType type, int count, size_t  *size);

extern void QCArrayXORAt(QCArrayRef array, int index, int value);

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

#endif

#ifndef QCFOREACH

#define QCFOREACH(array, exp, TYPE) do { \
                        for (size_t i = 0; i < array->count; ++i) { \
                             TYPE *d = array->data; \
                             exp; \
                         } \
                    } while(0)

#endif

#endif //PQC_CRYPTO_QCARRAYPRIVATE_H
