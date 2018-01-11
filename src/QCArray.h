//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAY_H
#define PQC_CRYPTO_QCARRAY_H

#include <stdbool.h>
#include <stdint.h>

typedef struct QCArray * QCArrayRef;

/*
 *  create an array with `count`, zero filled
 */
extern QCArrayRef QCArrayCreate(int count);

/*
 * create an array with `count`, copy data from x
 */
extern QCArrayRef QCArrayCreateFrom(const double *x, int count);

/*
 * create an array with `count`, retain pointer x, no-copy data
 */
extern QCArrayRef QCArrayCreateNoCopy(double *x, int count, bool needfree);

/*
 * reset the count in array, this will not release memory
 */
extern void QCArraySetCount(QCArrayRef array, int newCount);

/*
 * set value at index
 */
extern void QCArraySetValueAt(QCArrayRef array, int index, double value);

/*
 * set fft flag
 */
extern void QCArraySetFFTFlag(QCArrayRef array, bool flag);

/*
 * do fft on `array`, return a new array
 */
extern QCArrayRef QCArrayFFT(QCArrayRef array);

/*
 * do inverse fft on `array`, return a new array
 */
extern QCArrayRef QCArrayInverseFFT(QCArrayRef array);

/*
 * scale each number in array (modified)
 */
extern void QCArrayScale(QCArrayRef array, double scale);

/*
 * do complex multiply : x * y, return a new array
 */
extern QCArrayRef QCArrayComplexMultiply(QCArrayRef x, QCArrayRef y);

/*
 * multiply each number in array by mul
 */
extern void QCArrayMultiply(QCArrayRef array, double mul);

/*
 * do `round` on each number in array
 */
extern void QCArrayRound(QCArrayRef array);

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
 * compare two array
 */
extern bool QCArrayCompare(QCArrayRef x, QCArrayRef expected);

/*
 * compare array with raw data
 */
extern bool QCArrayCompareRaw(QCArrayRef x, const double *expected);

/*
 * print an array
 */
extern void QCArrayPrint(QCArrayRef array);

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
