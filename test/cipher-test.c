//
// Created by Isaac on 2018/1/11.
//

#include <stddef.h>
#include <printf.h>
#include "cipher-test.h"
#include "src/QCCipher.h"
#include "src/QCKeyPrivate.h"
#include "data.h"

static void mul_poly_test() {
    printf("-----------mul poly test--------------\n");

    int length = 4801;
    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCArrayRef h0c0 = QCArrayMulPoly(h0, c0);
    QCArrayCompareRaw(h0c0, kMulPoly);

    QCArrayRef h1c1 = QCArrayMulPoly(h1, c1);
    QCArrayCompareRaw(h1c1, kH1C1MulPoly);

    QCArrayFree(h0);
    QCArrayFree(h1);
    QCArrayFree(c0);
    QCArrayFree(c1);
    QCArrayFree(h0c0);
    QCArrayFree(h1c1);

    printf("-----------mul poly test end--------------\n");
}

static void cipher_syndrome_test() {
    printf("-----------cipher syndrome test--------------\n");

    int length = 4801;
    int weight = 45;
    int error = 42;

    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);
    QCArrayRef h1inv = QCArrayCreateWithDouble(H1_inv, length, true);

    QCKeyRef privateKey = QCKeyCreateWith(h0, h1, h1inv, NULL, length, weight, error);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCArrayRef result = QCCipherSyndrome(privateKey, c0, c1);
    QCArrayCompareRaw(result, kSyndrome);

    QCArrayFree(h0);
    QCArrayFree(h1);
    QCArrayFree(h1inv);
    QCKeyFree(privateKey);
    QCArrayFree(c0);
    QCArrayFree(c1);
    QCArrayFree(result);
    printf("-----------cipher syndrome test end--------------\n");
}

static void decrypt_test() {
    printf("-----------decrypt test--------------\n");

    int length = 4801;
    int weight = 45;
    int error = 42;

    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);
    QCArrayRef h1inv = QCArrayCreateWithDouble(H1_inv, length, true);

    QCKeyRef privateKey = QCKeyCreateWith(h0, h1, h1inv, NULL, length, weight, error);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);
    QCArrayRef result = QCCipherDecrypt(privateKey, c0, c1);

    QCArrayCompareRaw(result, kQCMDPCDecrypt);

    QCArrayFree(h0);
    QCArrayFree(h1);
    QCArrayFree(h1inv);
    QCKeyFree(privateKey);
    QCArrayFree(c0);
    QCArrayFree(c1);
    QCArrayFree(result);

    printf("-----------decrypt test end--------------\n");
}

void cipher_test() {
    mul_poly_test();

    cipher_syndrome_test();

    decrypt_test();
}