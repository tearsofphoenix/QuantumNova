//
// Created by Isaac on 2018/1/11.
//

#include <stddef.h>
#include <printf.h>
#include <memory.h>
#include "cipher-test.h"
#include "src/QCCipher.h"
#include "src/QCKeyPrivate.h"
#include "src/vendor/aes.h"
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

static void print_hex(BYTE str[], int len)
{
    int idx;

    for(idx = 0; idx < len; idx++)
        printf("%02x", str[idx]);
}

static void aes_cbc_test()
{
    WORD key_schedule[60];
    BYTE enc_buf[128];
    BYTE plaintext[1][32] = {
            {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51}
    };
    BYTE ciphertext[1][32] = {
            {0xf5,0x8c,0x4c,0x04,0xd6,0xe5,0xf1,0xba,0x77,0x9e,0xab,0xfb,0x5f,0x7b,0xfb,0xd6,0x9c,0xfc,0x4e,0x96,0x7e,0xdb,0x80,0x8d,0x67,0x9f,0x77,0x7b,0xc6,0x70,0x2c,0x7d}
    };
    BYTE iv[1][16] = {
            {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}
    };
    BYTE key[1][32] = {
            {0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4}
    };
    int pass = 1;

    //printf("* CBC mode:\n");
    aes_key_setup(key[0], key_schedule, 256);

    printf(  "Key          : ");
    print_hex(key[0], 32);
    printf("\nIV           : ");
    print_hex(iv[0], 16);

    aes_encrypt_cbc(plaintext[0], 32, enc_buf, key_schedule, 256, iv[0]);
    printf("\nPlaintext    : ");
    print_hex(plaintext[0], 32);
    printf("\n-encrypted to: ");
    print_hex(enc_buf, 32);
    printf("\nCiphertext   : ");
    print_hex(ciphertext[0], 32);
    pass = pass && !memcmp(enc_buf, ciphertext[0], 32);

    aes_decrypt_cbc(ciphertext[0], 32, enc_buf, key_schedule, 256, iv[0]);
    printf("\nCiphertext   : ");
    print_hex(ciphertext[0], 32);
    printf("\n-decrypted to: ");
    print_hex(enc_buf, 32);
    printf("\nPlaintext   : ");
    print_hex(plaintext[0], 32);
    pass = pass && !memcmp(enc_buf, plaintext[0], 32);

    //printf("\n\n");
}

void cipher_test() {
    mul_poly_test();

    cipher_syndrome_test();

    decrypt_test();

    aes_cbc_test();
}