//
// Created by Isaac on 2018/1/10.
//

#include "fft-test.h"
#include "data.h"
#include "cipher-test.h"
#include <stdio.h>
#include <math.h>

typedef void (* QCTestFunc)(void);

static void fft_test() {
    printf("-----------FFT test--------------\n");

    size_t size = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef array = QCArrayCreateWithDouble(H0, size, true);

    QCArrayRef fft = QCArrayFFT(array);

    QCArrayRef real = QCArrayGetRealParts(fft);

    QCArrayRound(real);
    QCArrayCompareRaw(real, kH0FFTReal);

    QCRelease(array);
    QCRelease(fft);
    QCRelease(real);
}

static void complex_multiply_test() {
    printf("-----------complex multiply test--------------\n");
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef x = QCArrayFFT(tempH0);
    QCArrayRef tempC0 = QCArrayCreateWithDouble(C0, count, true);
    QCArrayRef y = QCArrayFFT(tempC0);
    QCArrayRef result = QCArrayComplexMultiply(x, y);
    QCArraySetCount(result, count);
    QCArrayRef real = QCArrayGetRealParts(result);
    QCArrayRound(real);
    QCArrayCompareRaw(real, kH0C0Multiply);

    QCRelease(tempH0);
    QCRelease(tempC0);
    QCRelease(result);
    QCRelease(real);
}

static void inverse_fft_test() {

    printf("-----------inverse FFT test--------------\n");
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef x = QCArrayFFT(tempH0);
    QCArrayRef result = QCArrayInverseFFT(x);
    QCArrayRound(result);
    QCArrayCompareRaw(result, H0);

    QCRelease(tempH0);
    QCRelease(x);
    QCRelease(result);
}

static void square_sparse_test() {
    printf("-----------square sparse test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);
    QCArrayRef array = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef tempH0 = QCArraySquareSparsePoly(array, 1);
    QCArrayCompareRaw(tempH0, kH0Sparse);

    QCRelease(array);
    QCRelease(tempH0);
}

static void mul_poly_test() {
    printf("-----------mul_poly test--------------\n");

    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef tempC0 = QCArrayCreateWithDouble(C0, count, true);
    QCArrayRef result = QCArrayMulPoly(tempH0, tempC0);

    QCArrayCompareRaw(result, kMulPoly);

    QCRelease(tempH0);
    QCRelease(tempC0);
    QCRelease(result);
}

static void exp_poly_test() {
    printf("-----------exp_poly test--------------\n");

    size_t count = sizeof(kExpInput) / sizeof(kExp20[0]);
    QCArrayRef tempH0 = QCArrayCreateWithDouble(kExpInput, count, true);
    int64_t n = pow(2, 20) - 2;
    QCArrayRef result = QCArrayExpPoly(tempH0, n);
    QCArrayCompareRaw(result, kExp20);
}

static void loop_test(QCTestFunc func, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        func();
    }
}

static void test_all(size_t count) {
    loop_test(fft_test, count);

    loop_test(complex_multiply_test, count);

    loop_test(inverse_fft_test, count);

    loop_test(square_sparse_test, count);

    loop_test(mul_poly_test, count);

    loop_test(exp_poly_test, count);
}

int main() {

//    test_all(1);

    cipher_test();

    return 0;
}