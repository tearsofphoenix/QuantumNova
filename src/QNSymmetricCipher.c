//
// Created by Isaac on 2018/1/22.
//

#include "QNSymmetricCipher.h"
#include "QCArrayPrivate.h"
#include "vendor/salsa20.h"
#include <tomcrypt.h>

// aes cipher
/*
 * symmetric encrypt (current AES-CBC)
 */
static QN_STRONG QCArrayRef QNAESEncrypt(QCArrayRef message, QCArrayRef key, QCArrayRef iv) {
    QCArrayRef padded = QCArrayPKCS7Encode(message);

    size_t messageSize = padded->count;
    QCByte *out = message->isa->allocator(messageSize * sizeof(QCByte));

    register_cipher(&aes_desc);
    symmetric_CBC cbc;
    int cipher_idx = find_cipher("aes");
    /* encode the block */
    int ret = cbc_start(cipher_idx, iv->data, key->data, key->count, 0, &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc start failed!\n");
        return NULL;
    }
    ret = cbc_encrypt(padded->data, out, messageSize * sizeof(QCByte), &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc encrypt failed!\n");
        return NULL;
    }

    QCArrayRef array = QCArrayCreateWithByte(out, messageSize, false);
    array->needfree = true;

    QCRelease(padded);

    return array;
}

/*
 * symmetric decrypt (current AES-CBC)
 */
static QN_STRONG QCArrayRef QNAESDecrypt(QCArrayRef message, QCArrayRef key, QCArrayRef iv) {
    size_t messageSize = message->count;
    QCByte *out = message->isa->allocator(messageSize * sizeof(QCByte));

    register_cipher(&aes_desc);
    symmetric_CBC cbc;
    int cipher_idx = find_cipher("aes");

    int ret = cbc_start(cipher_idx, iv->data, key->data, key->count, 0, &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc start failed!\n");
        return NULL;
    }
    ret = cbc_decrypt(message->data, out, messageSize * sizeof(QCByte), &cbc);
    if (ret != CRYPT_OK) {
        printf("cbc decrypt failed!\n");
        return NULL;
    }
    QCArrayRef array = QCArrayCreateWithByte(out, messageSize, false);
    array->needfree = true;

    QCArrayRef ref = QCArrayPKCS7Decode(array);
    QCRelease(array);
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
static QN_STRONG QCArrayRef QNSalsa20Encrypt(QCArrayRef message, QCArrayRef key, QCArrayRef iv) {
//    QCArrayRef padded = QCArrayPKCS7Encode(message);
    QCArrayRef copy = QCArrayCreateCopy(message);
    size_t size = QCArrayGetSize(copy);
    enum s20_status_t status = s20_crypt(key->data, S20_KEYLEN_256, iv->data, 0, copy->data, size);
    if (status == S20_SUCCESS) {
        return copy;
    } else {
        QCRelease(copy);
        return NULL;
    }
}

/*
 * symmetric decrypt (current AES-CBC)
 */
static QN_STRONG QCArrayRef QNSalsa20Decrypt(QCArrayRef message, QCArrayRef key, QCArrayRef iv) {
    QCArrayRef copy = QCArrayCreateCopy(message);
    size_t size = QCArrayGetSize(copy);
    enum s20_status_t status = s20_crypt(key->data, S20_KEYLEN_256, iv->data, 0, copy->data, size);
    if (status == S20_SUCCESS) {
        return copy;
    } else {
        QCRelease(copy);
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

