//
// Created by Isaac on 2018/1/22.
//

#include "QNSymmetricCipher.h"
#include "QNArrayPrivate.h"
#include "vendor/salsa20.h"
#include <tomcrypt.h>

// aes cipher
/*
 * symmetric encrypt (current AES-CBC)
 */
static QN_STRONG QNArrayRef QNAESEncrypt(QNArrayRef message, QNArrayRef key, QNArrayRef iv) {
    QNArrayRef padded = QNArrayPKCS7Encode(message);

    size_t messageSize = padded->count;
    QNByte *out = message->isa->allocator(messageSize * sizeof(QNByte));

    register_cipher(&aes_desc);
    symmetric_CBC cbc;
    int cipher_idx = find_cipher("aes");
    /* encode the block */
    int ret = cbc_start(cipher_idx, iv->data, key->data, key->count, 0, &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc start failed!\n");
        return NULL;
    }
    ret = cbc_encrypt(padded->data, out, messageSize * sizeof(QNByte), &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc encrypt failed!\n");
        return NULL;
    }

    QNArrayRef array = QNArrayCreateWithByte(out, messageSize, false);
    array->needfree = true;

    QNRelease(padded);

    return array;
}

/*
 * symmetric decrypt (current AES-CBC)
 */
static QN_STRONG QNArrayRef QNAESDecrypt(QNArrayRef message, QNArrayRef key, QNArrayRef iv) {
    size_t messageSize = message->count;
    QNByte *out = message->isa->allocator(messageSize * sizeof(QNByte));

    register_cipher(&aes_desc);
    symmetric_CBC cbc;
    int cipher_idx = find_cipher("aes");

    int ret = cbc_start(cipher_idx, iv->data, key->data, key->count, 0, &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc start failed!\n");
        return NULL;
    }
    ret = cbc_decrypt(message->data, out, messageSize * sizeof(QNByte), &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc decrypt failed!\n");
        return NULL;
    }
    QNArrayRef array = QNArrayCreateWithByte(out, messageSize, false);
    array->needfree = true;

    QNArrayRef ref = QNArrayPKCS7Decode(array);
    QNRelease(array);
    return ref;
}

static struct QNSymmetricCipher kAESCipher = {
        .encrypt = QNAESEncrypt,
        .decrypt = QNAESDecrypt,
};

QNSymmetricCipherRef QNGetAESCipher() {
    return &kAESCipher;
}

// salsa20 cipher
static QN_STRONG QNArrayRef QNSalsa20Encrypt(QNArrayRef message, QNArrayRef key, QNArrayRef iv) {
//    QNArrayRef padded = QNArrayPKCS7Encode(message);
    QNArrayRef copy = QNArrayCreateCopy(message);
    size_t size = QNArrayGetSize(copy);
    enum s20_status_t status = s20_crypt(key->data, S20_KEYLEN_256, iv->data, 0, copy->data, size);
    if (status == S20_SUCCESS) {
        return copy;
    } else {
        QNRelease(copy);
        return NULL;
    }
}

/*
 * symmetric decrypt (current AES-CBC)
 */
static QN_STRONG QNArrayRef QNSalsa20Decrypt(QNArrayRef message, QNArrayRef key, QNArrayRef iv) {
    QNArrayRef copy = QNArrayCreateCopy(message);
    size_t size = QNArrayGetSize(copy);
    enum s20_status_t status = s20_crypt(key->data, S20_KEYLEN_256, iv->data, 0, copy->data, size);
    if (status == S20_SUCCESS) {
        return copy;
    } else {
        QNRelease(copy);
        return NULL;
    }
}

static struct QNSymmetricCipher kSalsa20Cipher = {
        .encrypt = QNSalsa20Encrypt,
        .decrypt = QNSalsa20Decrypt,
};

QNSymmetricCipherRef QNGetSalsa20Cipher() {
    return &kSalsa20Cipher;
}

