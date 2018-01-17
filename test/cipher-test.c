//
// Created by Isaac on 2018/1/11.
//

#include <stddef.h>
#include <stdio.h>
#include <memory.h>
#include <src/QCKey.h>
#include <src/QCCipherPrivate.h>
#include "cipher-test.h"
#include "src/QCCipher.h"
#include "src/QCRandom.h"
#include "src/QCMessagePrivate.h"
#include "src/QCKeyPrivate.h"
#include "data.h"


QCKeyConfig config = {
        .length = 4801,
        .weight = 45,
        .error = 42
};

static QCKeyRef _getPrivateKey() {
    size_t length = config.length;
    QCArrayRef h0 = QCArrayCreateWithDouble(H0, length, true);
    QCArrayRef h1 = QCArrayCreateWithDouble(H1, length, true);
    QCArrayRef h1inv = QCArrayCreateWithDouble(H1_inv, length, true);

    QCKeyRef privateKey = QCKeyCreatePrivate(h0, h1, h1inv, config);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);

    return privateKey;
}

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

    size_t length = config.length;

    QCKeyRef privateKey = _getPrivateKey();

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);

    QCArrayRef result = QCCipherSyndrome(cipher, c0, c1);
    QCArrayCompareRaw(result, kSyndrome, QCDTDouble);

    QCRelease(privateKey);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(result);
    printf("-----------cipher syndrome test end--------------\n");
}

static void decrypt_test() {
    printf("-----------decrypt test--------------\n");
    size_t length = config.length;

    QCKeyRef privateKey = _getPrivateKey();

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);

    QCArrayRef result = QCCipherDecrypt(cipher, c0, c1);

    QCArrayCompareRaw(result, kQCMDPCDecrypt, QCDTDouble);

    QCRelease(privateKey);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(result);

    printf("-----------decrypt test end--------------\n");
}

static void decrypt_message_test() {
    QCByte stream[] = {0x5e, 0xca, 0x49, 0x4f, 0x5a, 0xb1, 0xf3, 0xd4, 0x8e, 0x1a, 0x37, 0xcd, 0x32, 0x77, 0xc6, 0x92,
                       0x2f, 0x47, 0x6e, 0x50, 0x7c, 0xcc, 0xa2, 0x68, 0x08, 0x68, 0x94, 0x4a, 0x73, 0x31, 0x70, 0x1f,
                       0x91, 0xd8, 0x4e, 0x0a, 0x62, 0x8b, 0x51, 0x92, 0xe7, 0x9d, 0xb1, 0x18, 0x28, 0x99, 0x73, 0x6d};
    QCByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QCArrayRef streamArray = QCArrayCreateWithByte(stream, sizeof(stream) / sizeof(QCByte), true);

    size_t length = config.length;

    QCKeyRef privateKey = _getPrivateKey();

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);
    QCMessageRef message = QCMessageCreate(c0, c1, streamArray);
    QCArrayRef array = QCCipherDecryptMessage(cipher, message);

    if (QCArrayCompareRaw(array, msg, QCDTByte) ) {
        printf("cipher decrypt test passed\n");
    }

    QCRelease(c0);
    QCRelease(c1);
    QCRelease(cipher);
    QCRelease(message);
    QCRelease(array);
    QCRelease(privateKey);
}

static void encrypt_test() {
    printf("-----------encrypt test start--------------\n");

    size_t length = config.length;
    QCByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QCKeyRef privateKey = _getPrivateKey();

    QCArrayRef c0 = QCArrayCreateWithDouble(C0, length, true);
    QCArrayRef c1 = QCArrayCreateWithDouble(C1, length, true);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);

    QCArrayRef g = QCArrayCreateWithDouble(G, length, true);
    QCKeyRef publicKey = QCKeyCreatePublic(g, config);
    QCCipherSetPublicKey(cipher, publicKey);

    QCArrayRef stream = QCArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QCMessageRef enc = QCCipherEncryptMessage(cipher, stream);
    enc->c0 = c0;
    enc->c1 = c1;
    QCArrayRef array = QCCipherDecryptMessage(cipher, enc);
    if (QCArrayCompareRaw(array, msg, QCDTByte) ) {
        printf("cipher encrypt test passed\n");
    }

    QCRelease(privateKey);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(stream);
    QCRelease(enc);
    QCRelease(array);
    QCRelease(publicKey);
    QCRelease(cipher);

    printf("-----------encrypt test end--------------\n");
}

static void aes_cbc_test()
{
    printf("-----------aes cbc test--------------\n");
    QCByte plaintext[1][32] = {
            {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51}
    };
    QCByte ciphertext[1][32] = {
            {0xf5,0x8c,0x4c,0x04,0xd6,0xe5,0xf1,0xba,0x77,0x9e,0xab,0xfb,0x5f,0x7b,0xfb,0xd6,0x9c,0xfc,0x4e,0x96,0x7e,0xdb,0x80,0x8d,0x67,0x9f,0x77,0x7b,0xc6,0x70,0x2c,0x7d}
    };
    QCByte iv[1][16] = {
            {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}
    };
    QCByte key[1][32] = {
            {0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4}
    };

    QCCipherRef cipher = QCCipherCreate();

    QCArrayRef message = QCArrayCreateWithByte(plaintext[0], 32, false);
    QCArrayRef keyArray = QCArrayCreateWithByte(key[0], 32, false);
    QCArrayRef ivArray = QCArrayCreateWithByte(iv[0], 16, false);

    QCArrayRef ciphered = QCCipherSymmetricEncrypt(cipher, message, keyArray, ivArray);

    if(QCArrayCompareRaw(ciphered, ciphertext[0], QCDTByte)) {
        printf("passed aes encrypt\n");
    }

    QCArrayRef plain = QCCipherSymmetricDecrypt(cipher, ciphered, keyArray, ivArray);

    if (QCObjectEqual(plain, message)) {
        printf("passed aes decrypt\n");
    }

    QCRelease(cipher);
    QCRelease(message);
    QCRelease(keyArray);
    QCRelease(ivArray);
    QCRelease(plain);
    QCRelease(ciphered);
}

static void mac_test() {
    QCByte message[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};
    QCByte token[] = {0xee, 0xc7, 0xf5, 0xa7, 0x9b, 0x1b, 0x42, 0x2a, 0x7c, 0x32, 0xe0, 0xe4, 0xbe, 0xcc, 0x50, 0x7f,
                      0x66, 0x9f, 0x88, 0x0a, 0x86, 0x3b, 0xbb, 0x71, 0xb7, 0xe7, 0xd0, 0x81, 0x43, 0x54, 0xec, 0xab,
                      0x1e, 0x36, 0x44, 0x51, 0xc4, 0x7d, 0x91, 0xcf, 0x3d, 0xf2, 0x92, 0xe4, 0xc6, 0x49, 0xb5, 0xcd,
                      0xe6, 0x89, 0x0a, 0xf5, 0x30, 0x49, 0xd1, 0xe7, 0x5c, 0xf8, 0x26, 0x72, 0x01, 0x1f, 0x24, 0x99};
    QCByte keyB[] = {0x7d, 0x25, 0x0b, 0x7b, 0x25, 0xc4, 0xdf, 0x9a, 0xe1, 0x1e, 0x0f, 0xab, 0xea, 0xa5, 0x41, 0x7a,
                     0x70, 0x99, 0x8f, 0x16, 0x03, 0x4a, 0x02, 0xca, 0x27, 0xd1, 0x4a, 0x93, 0xe0, 0xc2, 0x87, 0x5c};
    QCByte mac[] = {0xc4, 0x70, 0xe4, 0x73, 0xc6, 0x2e, 0xfb, 0xfe, 0x62, 0x70, 0x8d, 0x2e, 0x51, 0xa2, 0x43, 0x51,
                    0x97, 0xce, 0xdf, 0x9e, 0x95, 0x73, 0xd0, 0xf0, 0x6f, 0xb0, 0x79, 0x18, 0xb1, 0x97, 0x69, 0x35};
    QCArrayRef m = QCArrayCreateWithByte(message, sizeof(message), true);
    QCArrayRef t = QCArrayCreateWithByte(token, sizeof(token), false);
    QCArrayRef k = QCArrayCreateWithByte(keyB, sizeof(keyB), false);
    QCArrayAppend(m, t);
    QCArrayAppend(m, k);

    QCArrayRef h = QCArraySHA256(m);

    if (QCArrayCompareRaw(h, mac, QCDTByte)) {
        printf("mac test passed\n");
    }

    QCRelease(m);
    QCRelease(t);
    QCRelease(k);
    QCRelease(h);
}

static void file_test() {
    printf("-------------file test start-------------\n");
    const char *privateKeyPath = "./aux/priv.key";
    const char *publicKeyPath = "./aux/pub.key";

    QCByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QCKeyRef privateKey = QCKeyCreateFromPEMFile(privateKeyPath);
    QCKeyRef publicKey = QCKeyCreateFromPEMFile(publicKeyPath);

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privateKey);
    QCCipherSetPublicKey(cipher, publicKey);

    QCArrayRef stream = QCArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QCMessageRef enc = QCCipherEncryptMessage(cipher, stream);

    QCArrayRef array = QCCipherDecryptMessage(cipher, enc);
    if (QCArrayCompareRaw(array, msg, QCDTByte) ) {
        printf("cipher encrypt test passed\n");
    }

    QCRelease(privateKey);
    QCRelease(publicKey);
    QCRelease(stream);
    QCRelease(enc);
    QCRelease(array);
    QCRelease(cipher);
}

void cipher_test() {
    mul_poly_test();

    cipher_syndrome_test();

    decrypt_test();

    aes_cbc_test();

    mac_test();

    encrypt_test();

    decrypt_message_test();

//    file_test();
}