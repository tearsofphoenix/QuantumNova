//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAY_H
#define PQC_CRYPTO_QCARRAY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "QCClass.h"

typedef enum {
    QCDTInt = 0,
    QCDTByte,
    QCDTDouble,
} QCArrayDataType;

typedef struct QCArray * QCArrayRef;

/*
 *  create an array with `count`, zero filled
 */
extern QCArrayRef QCArrayCreate(int count);

/*
 * create QCArray from double array
 */
extern QCArrayRef QCArrayCreateWithDouble(const double *array, size_t count, bool needCopy);

/*
 * create QCArray from int32 array
 */
extern QCArrayRef QCArrayCreateWithInt(const int *array, size_t count, bool needCopy);

/*
 * create QCArray from byte array
 */
extern QCArrayRef QCArrayCreateWithByte(const QCByte *array, size_t count, bool needCopy);

/*
 * create copy of array
 */
extern QCArrayRef QCArrayCreateCopy(QCArrayRef array);

/*
 * reset the count in array, this will not release memory
 */
extern void QCArraySetCount(QCArrayRef array, size_t newCount);

/*
 *
 */
extern QCArrayRef QCArraySHA256(QCArrayRef array);

/*
 * set value at index
 */
extern void QCArraySetValueAt(QCArrayRef array, int index, double value);

/*
 * get value at index
 */
extern double QCArrayValueAt(QCArrayRef array, int index);

/*
 * find first index of value
 */
extern int QCArrayFindIndex(QCArrayRef array, int value);

/*
 * get nonzero number count
 */

extern int QCArrayGetNonZeroCount(QCArrayRef array);

/*
 * for-each loop on array
 */
extern void QCArrayForeach(QCArrayRef array, const void *func, const void *ctx);

/*
 * do fft on `array`, return a new array
 */
extern QCArrayRef QCArrayFFT(QCArrayRef array);

/*
 * do inverse fft on `array`, return a new array
 */
extern QCArrayRef QCArrayInverseFFT(QCArrayRef array);

/*
 * do complex multiply : x * y, return a new array
 */
extern QCArrayRef QCArrayComplexMultiply(QCArrayRef x, QCArrayRef y);

/*
 * add each number of two arrays
 */
extern void QCArrayAddArray(QCArrayRef x, QCArrayRef y);

/*
 * add each number of two arrays
 */
extern void QCArrayAddAt(QCArrayRef x, int index, double value);

/*
 * multiply each number in array by mul
 */
extern void QCArrayMultiply(QCArrayRef array, double mul);

/*
 * do `round` on each number in array
 */
extern void QCArrayRound(QCArrayRef array);

/*
 * get max value in array
 */
extern double QCArrayMax(QCArrayRef array);

/*
 * do `mod` on each number in array, result store in array
 */
extern void QCArrayMod(QCArrayRef array, int mod);

/*
 * return real parts of each complex number in array
 */
extern QCArrayRef QCArrayGetRealParts(QCArrayRef complexArray);

/*
 * return indices of no-zero number in array
 */
extern QCArrayRef QCArrayGetNoZeroIndices(QCArrayRef array);

/*
 * compare array with raw data
 */
extern bool QCArrayCompareRaw(QCArrayRef x, const void *expected, QCArrayDataType dataType);

/*
 * square spare poly
 */
extern QCArrayRef QCArraySquareSparsePoly(QCArrayRef array, int times);

/*
 * mul poly
 */
extern QCArrayRef QCArrayMulPoly(QCArrayRef x, QCArrayRef y);

/*
 * exp poly
 */
extern QCArrayRef QCArrayExpPoly(QCArrayRef array, int64_t n);

/*
 * free array's data
 */
extern void QCArrayDeallocate(QCArrayRef array);


#endif //PQC_CRYPTO_QCARRAY_H
