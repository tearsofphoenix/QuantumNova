//
// Created by Isaac on 2018/1/10.
//

#include "fft-test.h"
#include "data.h"
#include <stdio.h>

static void fft_test() {
    printf("-----------FFT test--------------\n");

    int size = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef array = QCArrayCreateFrom(H0, size);
    QCArrayRef fft = QCArrayFFT(array);

    QCArrayRef real = QCArrayGetRealParts(fft);
    QCArrayRound(real);
    QCArrayCompareRaw(real, kH0FFTReal);

    QCArrayFree(array);
    QCArrayFree(fft);
    QCArrayFree(real);
}

static void complex_multiply_test() {
    printf("-----------complex multiply test--------------\n");
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateFrom(H0, count);
    QCArrayRef x = QCArrayFFT(tempH0);
    QCArrayRef tempC0 = QCArrayCreateFrom(C0, count);
    QCArrayRef y = QCArrayFFT(tempC0);
    QCArrayRef result = QCArrayComplexMultiply(x, y);
    QCArraySetCount(result, count);
    QCArrayRef real = QCArrayGetRealParts(result);
    QCArrayRound(real);
    QCArrayCompareRaw(real, kH0C0Multiply);

    QCArrayFree(tempH0);
    QCArrayFree(tempC0);
    QCArrayFree(result);
    QCArrayFree(real);
}

static void inverse_fft_test() {

    printf("-----------inverse FFT test--------------\n");
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateFrom(H0, count);
    QCArrayRef x = QCArrayFFT(tempH0);
    QCArrayRef result = QCArrayInverseFFT(x);
    QCArrayRound(result);
    QCArrayCompareRaw(result, H0);

    QCArrayFree(tempH0);
    QCArrayFree(x);
    QCArrayFree(result);
}

static void square_sparse_test() {
    printf("-----------square sparse test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);
    QCArrayRef array = QCArrayCreateFrom(H0, count);
    QCArrayRef tempH0 = QCArraySquareSparsePoly(array, 1);
    QCArrayCompareRaw(tempH0, kH0Sparse);

    QCArrayFree(array);
    QCArrayFree(tempH0);
}

static void mul_poly_test() {
    printf("-----------mul_poly test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateFrom(H0, count);
    QCArrayRef tempC0 = QCArrayCreateFrom(C0, count);
    QCArrayRef result = QCArrayMulPoly(tempH0, tempC0);

    QCArrayCompareRaw(result, kMulPolyOrigin);
    QCArrayPrint(result);

    QCArrayFree(tempH0);
    QCArrayFree(tempC0);
    QCArrayFree(result);
}

static void exp_poly_test() {
    printf("-----------exp_poly test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);
    QCArrayRef tempH0 = QCArrayCreateFrom(H0, count);
}

int main() {

//    fft_test();

//    complex_multiply_test();

//    inverse_fft_test();

//    square_sparse_test();

    mul_poly_test();

    return 0;
}