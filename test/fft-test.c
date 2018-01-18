//
// Created by Isaac on 2018/1/10.
//

#include "fft-test.h"
#include "data.h"
#include "src/QCArrayPrivate.h"
#include "src/QNTest.h"
#include "src/QCObjectPrivate.h"

static bool fft_test() {
    size_t size = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef array = QCArrayCreateWithDouble(H0, size, true);
    QCArrayRef fft = QCArrayFFT(array);
    QCArrayRef real = QCArrayGetRealParts(fft);
    QCArrayRound(real);
    bool result = QCArrayCompareRaw(real, kH0FFTReal, QCDTDouble);

    QCRelease(array);
    QCRelease(fft);
    QCRelease(real);

    _QCPrintMemoryLeak();

    return result;
}

static bool complex_multiply_test() {
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef x = QCArrayFFT(tempH0);
    QCArrayRef tempC0 = QCArrayCreateWithDouble(C0, count, true);
    QCArrayRef y = QCArrayFFT(tempC0);
    QCArrayRef result = QCArrayComplexMultiply(x, y);
    QCArraySetCount(result, count);
    QCArrayRef real = QCArrayGetRealParts(result);
    QCArrayRound(real);

    bool ret = QCArrayCompareRaw(real, kH0C0Multiply, QCDTDouble);

    QCRelease(tempH0);
    QCRelease(tempC0);
    QCRelease(result);
    QCRelease(real);

    return ret;
}

static bool inverse_fft_test() {
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef x = QCArrayFFT(tempH0);
    QCArrayRef result = QCArrayInverseFFT(x);
    QCArrayRound(result);
    bool ret = QCArrayCompareRaw(result, H0, QCDTDouble);

    QCRelease(tempH0);
    QCRelease(x);
    QCRelease(result);
    return ret;
}

static bool square_sparse_test() {

    size_t count = sizeof(H0) / sizeof(H0[0]);
    QCArrayRef array = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef tempH0 = QCArraySquareSparsePoly(array, 1);
    bool ret = QCArrayCompareRaw(tempH0, kH0Sparse, QCDTDouble);

    QCRelease(array);
    QCRelease(tempH0);

    return ret;
}

static bool mul_poly_test() {

    size_t count = sizeof(H0) / sizeof(H0[0]);

    QCArrayRef tempH0 = QCArrayCreateWithDouble(H0, count, true);
    QCArrayRef tempC0 = QCArrayCreateWithDouble(C0, count, true);
    QCArrayRef result = QCArrayMulPoly(tempH0, tempC0);

    bool ret = QCArrayCompareRaw(result, kMulPoly, QCDTDouble);

    QCRelease(tempH0);
    QCRelease(tempC0);
    QCRelease(result);

    return ret;
}

static bool exp_poly_test() {
    size_t count = sizeof(kExpInput) / sizeof(kExp20[0]);
    QCArrayRef tempH0 = QCArrayCreateWithDouble(kExpInput, count, true);

    BN_CTX *bnCTX = BN_CTX_new();
    BIGNUM *base = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *n = BN_new();

    BN_dec2bn(&base, "2");
    BN_dec2bn(&exp, "20");
    BN_exp(n, base, exp, bnCTX);
    BN_sub(n, n, base);

    QCArrayRef result = QCArrayExpPoly(tempH0, n);
    bool ret = QCArrayCompareRaw(result, kExp20, QCDTDouble);

    BN_free(base);
    BN_free(exp);
    BN_free(n);
    BN_CTX_free(bnCTX);

    return ret;
}

void fft_test_all() {
    size_t count = 1;

    QNT("fft", "", fft_test, count);


//    QNT("complex multiply", "", complex_multiply_test, count);
//
//    QNT("inverse fft", "", inverse_fft_test, count);
//
//    QNT("square sparse", "", square_sparse_test, count);
//
//    QNT("mul_poly", "", mul_poly_test, count);
//
//    QNT("exp_poly", "", exp_poly_test, count);
}
