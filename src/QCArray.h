//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAY_H
#define PQC_CRYPTO_QCARRAY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum {
    QCDTInt = 0,
    QCDTFloat,
    QCDTDouble,
} QCArrayDataType;

typedef void (QCArrayEnumerator)(double num, int index, const void *ctx);

typedef struct QCArray * QCArrayRef;

/*
 *  create an array with `count`, zero filled
 */
extern QCArrayRef QCArrayCreate(int count);

/*
 *
 */
extern QCArrayRef QCArrayCreateWithDouble(const double *array, size_t count, bool needCopy);

extern QCArrayRef QCArrayCreateWithInt(const int *array, size_t count, bool needCopy);

/*
 * create copy of array
 */
extern QCArrayRef QCArrayCreateCopy(QCArrayRef array);

/*
 * reset the count in array, this will not release memory
 */
extern void QCArraySetCount(QCArrayRef array, int newCount);

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
extern void QCArrayForeach(QCArrayRef array, QCArrayEnumerator func, const void *ctx);

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
extern bool QCArrayCompareRaw(QCArrayRef x, const double *expected);

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
extern void QCArrayFree(QCArrayRef array);


#endif //PQC_CRYPTO_QCARRAY_H