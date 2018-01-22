//
// Created by Isaac on 2018/1/11.
//

#ifndef QN_QNARRAY_H
#define QN_QNARRAY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "QNClass.h"

typedef enum {
    QNDTInt = 0,
    QNDTByte,
    QNDTDouble,
} QNArrayDataType;

typedef struct QNArray * QNArrayRef;

/*
 *  create an array with `count`, zero filled
 */
extern QN_STRONG QNArrayRef QNArrayCreate(int count);

/*
 * create QNArray from double array
 */
extern QN_STRONG QNArrayRef QNArrayCreateWithDouble(const double *array, size_t count, bool needCopy);

/*
 * create QNArray from int32 array
 */
extern QN_STRONG QNArrayRef QNArrayCreateWithInt(const int *array, size_t count, bool needCopy);

/*
 * create QNArray from byte array
 */
extern QN_STRONG QNArrayRef QNArrayCreateWithByte(const QNByte *array, size_t count, bool needCopy);

/*
 * create QNArray from hex string
 */
extern QN_STRONG QNArrayRef QNArrayCreateWithHex(const char *hexString, size_t length);

/*
 * create QNArray from base64 string
 */
extern QN_STRONG QNArrayRef QNArrayCreateWithBase64(const char *base64String, size_t length);

/*
 * load array from file
 */
extern QN_STRONG QNArrayRef QNArrayFromFile(FILE *fp);

/*
 * save array to file
 */
extern QN_STRONG bool QNArraySaveToFile(QNArrayRef array, FILE *fp);

/*
 * do PKCS7 encode on array
 */
extern QNArrayRef QNArrayPKCS7Encode(QNArrayRef array);
/*
 * do PKCS7 decode on array
 */
extern QNArrayRef QNArrayPKCS7Decode(QNArrayRef array);

/*
 * create copy of array
 */
extern QN_STRONG QNArrayRef QNArrayCreateCopy(QNArrayRef array);

/*
 * reset the count in array, this will not release memory
 */
extern void QNArraySetCount(QNArrayRef array, size_t newCount);

/*
 * do sha256 on byte array
 */
extern QN_STRONG QNArrayRef QNArraySHA256(QNArrayRef byteArray);

/*
 * do sha512 on byte array
 */
extern QN_STRONG QNArrayRef QNArraySHA512(QNArrayRef byteArray);

/*
 * do sha512 on byte array
 */
extern QN_STRONG QNArrayRef QNArrayPack(QNArrayRef byteArray);

/*
 * append array
 */
extern void QNArrayAppend(QNArrayRef array, QNArrayRef other);

/*
 * slice an array
 */
extern QN_STRONG QNArrayRef QNArraySlice(QNArrayRef array, size_t start, size_t end);

/*
 * convert array to another typed array
 */
extern QN_STRONG QNArrayRef QNArrayConvert(QNArrayRef array, QNArrayDataType type);

/*
 * set value at index
 */
extern void QNArraySetValueAt(QNArrayRef array, int index, double value);

/*
 * get value at index
 */
extern double QNArrayValueAt(QNArrayRef array, int index);

/*
 * find first index of value
 */
extern int QNArrayFindIndex(QNArrayRef array, int value);

/*
 * get nonzero number count
 */
extern int QNArrayGetNonZeroCount(QNArrayRef array);

/*
 *
 */
extern size_t QNArrayGetSize(QNArrayRef array);

/*
 * for-each loop on array
 */
extern void QNArrayForeach(QNArrayRef array, const void *func, const void *ctx);

/*
 * do fft on `array`, return a new array
 */
extern QN_STRONG QNArrayRef QNArrayFFT(QNArrayRef array);

/*
 * do inverse fft on `array`, return a new array
 */
extern QN_STRONG QNArrayRef QNArrayInverseFFT(QNArrayRef array);

/*
 * do complex multiply : x * y, return a new array
 */
extern QN_STRONG QNArrayRef QNArrayComplexMultiply(QNArrayRef x, QNArrayRef y);

/*
 * add each number of two arrays
 */
extern void QNArrayAddArray(QNArrayRef x, QNArrayRef y);

/*
 * add each number of two arrays
 */
extern void QNArrayAddAt(QNArrayRef x, int index, double value);

/*
 * do xor on index-th number in array
 */
extern void QNArrayXORAt(QNArrayRef array, int index, int value);

/*
 * multiply each number in array by mul
 */
extern void QNArrayMultiply(QNArrayRef array, double mul);

/*
 * do `round` on each number in array
 */
extern void QNArrayRound(QNArrayRef array);

/*
 * get max value in array
 */
extern double QNArrayMax(QNArrayRef array);

/*
 * do `mod` on each number in array, result store in array
 */
extern void QNArrayMod(QNArrayRef array, int mod);

/*
 * return real parts of each complex number in array
 */
extern QN_STRONG QNArrayRef QNArrayGetRealParts(QNArrayRef complexArray);

/*
 * return indices of no-zero number in array
 */
extern QN_STRONG QNArrayRef QNArrayGetNoZeroIndices(QNArrayRef array);

/*
 * compare array with raw data
 */
extern bool QNArrayCompareRaw(QNArrayRef x, const void *expected, QNArrayDataType dataType);

/*
 * square spare poly
 */
extern QN_STRONG QNArrayRef QNArraySquareSparsePoly(QNArrayRef array, int times);

/*
 * mul poly
 */
extern QN_STRONG QNArrayRef QNArrayMulPoly(QNArrayRef x, QNArrayRef y);


#endif //QN_QNARRAY_H
