//
// Created by Isaac on 2018/1/10.
//

#include "fft-test.h"
#include "data.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static void fft_test() {
    printf("-----------FFT test--------------\n");

    int size = sizeof(H0) / sizeof(H0[0]);
    double *result = QCFFT(H0, size);
    double *real = QCGetRealParts(result, size);

    QCArrayRound(real, size);
    QCCompareArray(real, kH0FFTReal, size);

    fftw_free(result);
    fftw_free(real);
}

static void complex_multiply_test() {
    printf("-----------complex multiply test--------------\n");
    size_t count = sizeof(H0) / sizeof(H0[0]);

    double *tempH0 = QCFFT(H0, count);
    double *tempC0 = QCFFT(C0, count);
    double *result = QCComplexMultiply(tempH0, tempC0, count + 2);
    double *real = QCGetRealParts(result, count);

    QCArrayRound(real, count);
    QCCompareArray(real, kH0C0Multiply, count);

    fftw_free(tempH0);
    fftw_free(tempC0);
    fftw_free(result);
    fftw_free(real);
}

static void inverse_fft_test() {

    printf("-----------inverse FFT test--------------\n");
    size_t count = sizeof(H0) / sizeof(H0[0]);

    double *tempH0 = QCFFT(H0, count);
    double *result = QCInverseFFT(tempH0, count + 2);
    QCArrayRound(result, count);
    QCCompareArray(result, H0, count);

    fftw_free(tempH0);
    fftw_free(result);
}

static void square_sparse_test() {
    printf("-----------square sparse test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);

    double *tempH0 = QCSquareSparsePoly(H0, count, 1);
    QCCompareArray(tempH0, kH0Sparse, count);

    fftw_free(tempH0);
}

static void mul_poly_test() {
    printf("-----------mul_poly test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);

    double *result = QCMulPoly(H0, C0, count);

    QCCompareArray(result, kMulPoly, count);
//    QCPrintDoubleArray(result, count);

    fftw_free(result);
}

int main() {

    fft_test();

    complex_multiply_test();

    inverse_fft_test();

    square_sparse_test();

//    mul_poly_test();
}