//
// Created by Isaac on 2018/1/11.
//

#include <stddef.h>
#include <stdio.h>
#include <memory.h>
#include <src/QNKey.h>
#include <src/QNCipherPrivate.h>
#include <src/QNObjectPrivate.h>
#include "cipher-test.h"
#include "src/QNMessagePrivate.h"
#include "src/QNKeyPrivate.h"
#include "src/QNTest.h"
#include "src/QNSymmetricCipher.h"
#include "data.h"

static QN_STRONG QNKeyRef _getPrivateKey() {
    QNKeyConfig config = kQNDefaultKeyConfig;
    size_t length = config.length;
    QNArrayRef h0 = QNArrayCreateWithDouble(H0, length, true);
    QNArrayRef h1 = QNArrayCreateWithDouble(H1, length, true);
    QNArrayRef h1inv = QNArrayCreateWithDouble(H1_inv, length, true);

    QNKeyRef privateKey = QNKeyCreatePrivate(h0, h1, h1inv, config);

    QNRelease(h0);
    QNRelease(h1);
    QNRelease(h1inv);

    return privateKey;
}

static bool cipher_syndrome_test() {
    QNKeyConfig config = kQNDefaultKeyConfig;
    size_t length = config.length;

    QNKeyRef privateKey = _getPrivateKey();

    QNArrayRef c0 = QNArrayCreateWithDouble(C0, length, true);
    QNArrayRef c1 = QNArrayCreateWithDouble(C1, length, true);

    QNAsymmetricCipherRef asymmetricCipher = QNGetAsymmetricCipher();
    QNArrayRef result = asymmetricCipher->syndrome(privateKey, c0, c1);
    bool ret = QNArrayCompareRaw(result, kSyndrome, QNDTDouble);

    QNRelease(privateKey);
    QNRelease(c0);
    QNRelease(c1);
    QNRelease(result);

    return ret;
}

static bool decrypt_test() {
    QNKeyConfig config = kQNDefaultKeyConfig;
    size_t length = config.length;

    QNKeyRef privateKey = _getPrivateKey();

    QNArrayRef c0 = QNArrayCreateWithDouble(C0, length, true);
    QNArrayRef c1 = QNArrayCreateWithDouble(C1, length, true);

    QNCipherRef cipher = QNCipherCreate();
    QNCipherSetPrivateKey(cipher, privateKey);

    QNAsymmetricCipherRef asymmetricCipher = QNGetAsymmetricCipher();
    asymmetricCipher->decrypt(privateKey, c0, c1);
    QNArrayRef result = c0;
    bool ret = QNArrayCompareRaw(result, kQNMDPCDecrypt, QNDTDouble);

    QNRelease(privateKey);
    QNRelease(c0);
    QNRelease(c1);
    QNRelease(cipher);

    return ret;
}

static bool encrypt_test() {
    QNKeyConfig config = kQNDefaultKeyConfig;
    size_t length = config.length;
    QNByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QNKeyRef privateKey = _getPrivateKey();

    QNCipherRef cipher = QNCipherCreate();
    QNCipherSetPrivateKey(cipher, privateKey);

    QNArrayRef g = QNArrayCreateWithDouble(G, length, true);
    QNKeyRef publicKey = QNKeyCreatePublic(g, config);
    QNCipherSetPublicKey(cipher, publicKey);

    QNArrayRef stream = QNArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QNMessageRef enc = QNCipherEncryptMessage(cipher, stream);

    QNArrayRef array = QNCipherDecryptMessage(cipher, enc);
    bool ret = QNArrayCompareRaw(array, msg, QNDTByte);

    QNRelease(g);
    QNRelease(privateKey);
    QNRelease(stream);
    QNRelease(enc);
    QNRelease(array);
    QNRelease(publicKey);
    QNRelease(cipher);

    return ret;
}

static bool aes_cbc_test()
{
    QNByte plaintext[1][32] = {
            {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51}
    };
    QNByte ciphertext[1][32] = {
            {0xf5,0x8c,0x4c,0x04,0xd6,0xe5,0xf1,0xba,0x77,0x9e,0xab,0xfb,0x5f,0x7b,0xfb,0xd6,0x9c,0xfc,0x4e,0x96,0x7e,0xdb,0x80,0x8d,0x67,0x9f,0x77,0x7b,0xc6,0x70,0x2c,0x7d}
    };
    QNByte iv[1][16] = {
            {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}
    };
    QNByte key[1][32] = {
            {0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4}
    };

    QNArrayRef message = QNArrayCreateWithByte(plaintext[0], 32, false);
    QNArrayRef keyArray = QNArrayCreateWithByte(key[0], 32, false);
    QNArrayRef ivArray = QNArrayCreateWithByte(iv[0], 16, false);

    QNSymmetricCipherRef aesCipher = QNGetAESCipher();

    QNArrayRef ciphered = aesCipher->encrypt(message, keyArray, ivArray);

    bool ret1 = QNArrayCompareRaw(ciphered, ciphertext[0], QNDTByte);

    QNArrayRef plain = aesCipher->decrypt(ciphered, keyArray, ivArray);

    bool ret2 = QNObjectEqual(plain, message);

    QNRelease(message);
    QNRelease(keyArray);
    QNRelease(ivArray);
    QNRelease(plain);
    QNRelease(ciphered);

    return ret1 && ret2;
}

static bool salsa20_test()
{
    QNByte plaintext[1][32] = {
            {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51}
    };
    QNByte ciphertext[1][32] = {
            {0xf1, 0xfb, 0xe3, 0xa7, 0x0f, 0xef, 0xfc, 0x56, 0x5d, 0x4b, 0x26, 0xe4, 0x8a, 0x7c, 0x2e, 0x27, 0xbb, 0x20, 0x7e, 0x25, 0xb8, 0x08, 0x81, 0x62, 0xc4, 0x5a, 0xcf,
                    0x10, 0x65, 0xfc, 0xd5, 0xe2}
    };
    QNByte iv[1][16] = {
            {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}
    };
    QNByte key[1][32] = {
            {0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4}
    };

    QNArrayRef message = QNArrayCreateWithByte(plaintext[0], 32, false);
    QNArrayRef keyArray = QNArrayCreateWithByte(key[0], 32, false);
    QNArrayRef ivArray = QNArrayCreateWithByte(iv[0], 16, false);

    QNSymmetricCipherRef cipher = QNGetSalsa20Cipher();

    QNArrayRef ciphered = cipher->encrypt(message, keyArray, ivArray);

    bool ret1 = QNArrayCompareRaw(ciphered, ciphertext[0], QNDTByte);

    QNArrayRef plain = cipher->decrypt(ciphered, keyArray, ivArray);

    bool ret2 = QNObjectEqual(plain, message);

    QNRelease(message);
    QNRelease(keyArray);
    QNRelease(ivArray);
    QNRelease(plain);
    QNRelease(ciphered);

    return ret1 && ret2;
}

static bool mac_test() {
    QNByte message[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};
    QNByte token[] = {0xee, 0xc7, 0xf5, 0xa7, 0x9b, 0x1b, 0x42, 0x2a, 0x7c, 0x32, 0xe0, 0xe4, 0xbe, 0xcc, 0x50, 0x7f,
                      0x66, 0x9f, 0x88, 0x0a, 0x86, 0x3b, 0xbb, 0x71, 0xb7, 0xe7, 0xd0, 0x81, 0x43, 0x54, 0xec, 0xab,
                      0x1e, 0x36, 0x44, 0x51, 0xc4, 0x7d, 0x91, 0xcf, 0x3d, 0xf2, 0x92, 0xe4, 0xc6, 0x49, 0xb5, 0xcd,
                      0xe6, 0x89, 0x0a, 0xf5, 0x30, 0x49, 0xd1, 0xe7, 0x5c, 0xf8, 0x26, 0x72, 0x01, 0x1f, 0x24, 0x99};
    QNByte keyB[] = {0x7d, 0x25, 0x0b, 0x7b, 0x25, 0xc4, 0xdf, 0x9a, 0xe1, 0x1e, 0x0f, 0xab, 0xea, 0xa5, 0x41, 0x7a,
                     0x70, 0x99, 0x8f, 0x16, 0x03, 0x4a, 0x02, 0xca, 0x27, 0xd1, 0x4a, 0x93, 0xe0, 0xc2, 0x87, 0x5c};
    QNByte mac[] = {0xc4, 0x70, 0xe4, 0x73, 0xc6, 0x2e, 0xfb, 0xfe, 0x62, 0x70, 0x8d, 0x2e, 0x51, 0xa2, 0x43, 0x51,
                    0x97, 0xce, 0xdf, 0x9e, 0x95, 0x73, 0xd0, 0xf0, 0x6f, 0xb0, 0x79, 0x18, 0xb1, 0x97, 0x69, 0x35};
    QNArrayRef m = QNArrayCreateWithByte(message, sizeof(message), true);
    QNArrayRef t = QNArrayCreateWithByte(token, sizeof(token), true);
    QNArrayRef k = QNArrayCreateWithByte(keyB, sizeof(keyB), true);
    QNArrayAppend(m, t);
    QNArrayAppend(m, k);

    QNArrayRef h = QNArraySHA256(m);

    bool ret = QNArrayCompareRaw(h, mac, QNDTByte);

    QNRelease(m);
    QNRelease(t);
    QNRelease(k);
    QNRelease(h);

    return ret;
}

static bool file_test() {
    const char *privateKeyPath = "./aux/priv.key";
    const char *publicKeyPath = "./aux/pub.key";

    QNByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QNKeyRef privateKey = QNKeyCreateFromPEMFile(privateKeyPath);
    QNKeyRef publicKey = QNKeyCreateFromPEMFile(publicKeyPath);

    QNCipherRef cipher = QNCipherCreate();
    QNCipherSetPrivateKey(cipher, privateKey);
    QNCipherSetPublicKey(cipher, publicKey);

    QNArrayRef stream = QNArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QNMessageRef enc = QNCipherEncryptMessage(cipher, stream);

    QNArrayRef array = QNCipherDecryptMessage(cipher, enc);
    bool ret = QNArrayCompareRaw(array, msg, QNDTByte);

    QNRelease(privateKey);
    QNRelease(publicKey);
    QNRelease(stream);
    QNRelease(enc);
    QNRelease(array);
    QNRelease(cipher);

    return ret;
}

static bool _key_test(QNKeyConfig config1) {
    QNKeyRef privKey = NULL;
    QNKeyRef pubKey = NULL;
    QNKeyGeneratePair(config1, &privKey, &pubKey);

    QNByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QNCipherRef cipher = QNCipherCreate();
    QNCipherSetPrivateKey(cipher, privKey);
    QNCipherSetPublicKey(cipher, pubKey);

    QNArrayRef stream = QNArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QNMessageRef enc = QNCipherEncryptMessage(cipher, stream);

    QNArrayRef array = QNCipherDecryptMessage(cipher, enc);
    bool ret = QNArrayCompareRaw(array, msg, QNDTByte);

    QNRelease(privKey);
    QNRelease(stream);
    QNRelease(enc);
    QNRelease(array);
    QNRelease(pubKey);
    QNRelease(cipher);

    return ret;
}

static bool key_pair_test() {
    return _key_test(kQNDefaultKeyConfig);
}

static bool key_128bit_test() {
    return _key_test(kQN128BitKeyConfig);
}

static bool key_256bit_test() {
    return _key_test(kQN256BitKeyConfig);
}

void cipher_test() {
    QNT("cipher syndrome", NULL, cipher_syndrome_test, 1);

    QNT("cipher decrypt", NULL, decrypt_test, 1);

    QNT("aes cbc", NULL, aes_cbc_test, 1);

    QNT("salsa20 ", NULL, salsa20_test, 1);

    QNT("cipher mac", NULL, mac_test, 1);

    QNT("cipher encrypt", NULL, encrypt_test, 1);

//
//    QNT("key pair", NULL, key_pair_test, 1);

//    QNT("key(128bit) ", NULL, key_128bit_test, 1);

//    QNT("key(256bit) ", NULL, key_256bit_test, 1);

//    QNT("cipher file", NULL, file_test, 1);
}