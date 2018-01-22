//
// Created by Isaac on 2018/1/10.
//

#include "fft-test.h"
#include "data.h"
#include "src/QNArrayPrivate.h"
#include "src/QNTest.h"
#include "src/QNObjectPrivate.h"

static bool fft_test() {
    size_t size = sizeof(H0) / sizeof(H0[0]);

    QNArrayRef array = QNArrayCreateWithDouble(H0, size, true);
    QNArrayRef fft = QNArrayFFT(array);
    QNArrayRef real = QNArrayGetRealParts(fft);
    QNArrayRound(real);
    bool result = QNArrayCompareRaw(real, kH0FFTReal, QNDTDouble);

    QNRelease(array);
    QNRelease(fft);
    QNRelease(real);

    return result;
}

static bool complex_multiply_test() {
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QNArrayRef tempH0 = QNArrayCreateWithDouble(H0, count, true);
    QNArrayRef x = QNArrayFFT(tempH0);
    QNArrayRef tempC0 = QNArrayCreateWithDouble(C0, count, true);
    QNArrayRef y = QNArrayFFT(tempC0);
    QNArrayRef result = QNArrayComplexMultiply(x, y);
    QNArraySetCount(result, count);
    QNArrayRef real = QNArrayGetRealParts(result);
    QNArrayRound(real);

    bool ret = QNArrayCompareRaw(real, kH0C0Multiply, QNDTDouble);

    QNRelease(tempH0);
    QNRelease(tempC0);
    QNRelease(result);
    QNRelease(real);
    QNRelease(x);
    QNRelease(y);

    return ret;
}

static bool inverse_fft_test() {
    size_t count = sizeof(H0) / sizeof(H0[0]);

    QNArrayRef tempH0 = QNArrayCreateWithDouble(H0, count, true);
    QNArrayRef x = QNArrayFFT(tempH0);
    QNArrayRef result = QNArrayInverseFFT(x);
    QNArrayRound(result);
    bool ret = QNArrayCompareRaw(result, H0, QNDTDouble);

    QNRelease(tempH0);
    QNRelease(x);
    QNRelease(result);
    return ret;
}

static bool square_sparse_test() {

    size_t count = sizeof(H0) / sizeof(H0[0]);
    QNArrayRef array = QNArrayCreateWithDouble(H0, count, true);
    QNArrayRef tempH0 = QNArraySquareSparsePoly(array, 1);
    bool ret = QNArrayCompareRaw(tempH0, kH0Sparse, QNDTDouble);

    QNRelease(array);
    QNRelease(tempH0);

    return ret;
}

static bool mul_poly_test() {

    size_t count = sizeof(H0) / sizeof(H0[0]);

    QNArrayRef tempH0 = QNArrayCreateWithDouble(H0, count, true);
    QNArrayRef tempC0 = QNArrayCreateWithDouble(C0, count, true);
    QNArrayRef result = QNArrayMulPoly(tempH0, tempC0);

    bool ret = QNArrayCompareRaw(result, kMulPoly, QNDTDouble);

    QNRelease(tempH0);
    QNRelease(tempC0);
    QNRelease(result);

    return ret;
}

static bool exp_poly_test() {
    size_t count = sizeof(kExpInput) / sizeof(kExp20[0]);
    QNArrayRef tempH0 = QNArrayCreateWithDouble(kExpInput, count, true);

    BN_CTX *bnCTX = BN_CTX_new();
    BIGNUM *base = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *n = BN_new();

    BN_dec2bn(&base, "2");
    BN_dec2bn(&exp, "20");
    BN_exp(n, base, exp, bnCTX);
    BN_sub(n, n, base);

    QNArrayRef result = QNArrayExpPoly(tempH0, n);
    bool ret = QNArrayCompareRaw(result, kExp20, QNDTDouble);

    BN_free(base);
    BN_free(exp);
    BN_free(n);
    BN_CTX_free(bnCTX);

    QNRelease(tempH0);
    QNRelease(result);

    return ret;
}

void fft_test_all() {
    size_t count = 1;

    QNT("fft", "", fft_test, count);

    QNT("complex multiply", "", complex_multiply_test, count);

    QNT("inverse fft", "", inverse_fft_test, count);

    QNT("square sparse", "", square_sparse_test, count);

    QNT("mul_poly", "", mul_poly_test, count);

    QNT("exp_poly", "", exp_poly_test, count);
}
