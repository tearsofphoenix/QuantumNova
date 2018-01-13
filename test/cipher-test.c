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

    size_t length = 4801;
    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCArrayRef h0c0 = QCArrayMulPoly(h0, c0);
    QCArrayCompareRaw(h0c0, kMulPoly, QCDTDouble);

    QCArrayRef h1c1 = QCArrayMulPoly(h1, c1);
    QCArrayCompareRaw(h1c1, kH1C1MulPoly, QCDTDouble);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(h0c0);
    QCRelease(h1c1);

    printf("-----------mul poly test end--------------\n");
}

static void cipher_syndrome_test() {
    printf("-----------cipher syndrome test--------------\n");

    size_t length = 4801;
    size_t weight = 45;
    size_t error = 42;

    QCKeyConfig config = {
            .length = length,
            .weight = weight,
            .error = error
    };

    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);
    QCArrayRef h1inv = QCArrayCreateWithDouble(H1_inv, length, true);

    QCKeyRef privateKey = QCKeyCreateWith(h0, h1, h1inv, NULL, config);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);

    QCArrayRef result = QCCipherSyndrome(cipher, c0, c1);
    QCArrayCompareRaw(result, kSyndrome, QCDTDouble);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);
    QCRelease(privateKey);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(result);
    printf("-----------cipher syndrome test end--------------\n");
}

static void decrypt_test() {
    printf("-----------decrypt test--------------\n");

    size_t length = 4801;
    size_t weight = 45;
    size_t error = 42;

    QCKeyConfig config = {
            .length = length,
            .weight = weight,
            .error = error
    };

    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);
    QCArrayRef h1inv = QCArrayCreateWithDouble(H1_inv, length, true);

    QCKeyRef privateKey = QCKeyCreateWith(h0, h1, h1inv, NULL, config);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);

    QCArrayRef result = QCCipherDecrypt(cipher, c0, c1);

    QCArrayCompareRaw(result, kQCMDPCDecrypt, QCDTDouble);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);
    QCRelease(privateKey);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(result);

    printf("-----------decrypt test end--------------\n");
}

static void encrypt_test() {
    printf("-----------encrypt test start--------------\n");

    size_t length = 4801;
    size_t weight = 45;
    size_t error = 42;

    QCKeyConfig config = {
            .length = length,
            .weight = weight,
            .error = error
    };

    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);
    QCArrayRef h1inv = QCArrayCreateWithDouble(H1_inv, length, true);

    QCKeyRef privateKey = QCKeyCreateWith(h0, h1, h1inv, NULL, config);

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);

    QCArrayRef g = QCArrayCreateWithDouble(G, length, true);
    QCKeyRef publicKey = QCKeyCreateWith(NULL, NULL, NULL, g, config);
    QCCipherSetPublicKey(cipher, publicKey);

    QCCipherEncrypt(cipher, NULL, NULL, NULL);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);
    QCRelease(privateKey);
    QCRelease(c0);
    QCRelease(c1);

    printf("-----------encrypt test end--------------\n");
}

void cipher_test() {
    mul_poly_test();

    cipher_syndrome_test();

    decrypt_test();
}