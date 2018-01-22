//
// Created by Isaac on 2018/1/11.
//

#include "QNCipher.h"
#include "QNKeyPrivate.h"
#include "QNRandom.h"
#include "QNArrayPrivate.h"
#include "QNCipherPrivate.h"
#include "QNMessagePrivate.h"
#include <math.h>
#include <stdio.h>

static QNByte kSaltA[] = {"this is just a salt"};
static QNByte kSaltB[] = {"this is another a salt"};
static QNByte kIVSalt[] = {"third salt"};

static void QNCipherDeallocate(QNObjectRef object);

static struct QNClass kQNCipherClass = {
        .name = "QNCipher",
        .size = sizeof(struct QNCipher),
        .allocator = QNAllocator,
        .deallocate = QNCipherDeallocate
};

QNCipherRef QNCipherCreate(void) {
    QNCipherRef cipher = QNAllocate(&kQNCipherClass);
    cipher->symmetricCipher = QNGetSalsa20Cipher(); // default is AES
    cipher->asymmetricCipher = QNGetAsymmetricCipher();
    cipher->saltA = QNArrayCreateWithByte(kSaltA, strlen(kSaltA), false);
    cipher->saltB = QNArrayCreateWithByte(kSaltB, strlen(kSaltB), false);
    cipher->ivSalt = QNArrayCreateWithByte(kIVSalt, strlen(kIVSalt), false);
    return cipher;
}

QNKeyRef QNCipherGetPrivateKey(QNCipherRef cipher) {
    return cipher->privateKey;
}

QNKeyRef QNCipherGetPublicKey(QNCipherRef cipher) {
    return cipher->publicKey;
}

void QNCipherSetPrivateKey(QNCipherRef cipher, QNKeyRef privateKey) {
    QNRelease(cipher->privateKey);
    cipher->privateKey = QNRetain(privateKey);
}

void QNCipherSetPublicKey(QNCipherRef cipher, QNKeyRef publicKey) {
    QNRelease(cipher->publicKey);
    cipher->publicKey = QNRetain(publicKey);
}

static QN_STRONG QNArrayRef _sha256Contact(QNArrayRef a1, QNArrayRef a2, QNArrayRef a3) {
    QNArrayRef data = QNArrayCreateCopy(a1);
    if (a2) {
        QNArrayAppend(data, a2);
    }
    if (a3) {
        QNArrayAppend(data, a3);
    }
    QNArrayRef result = QNArraySHA256(data);
    QNRelease(data);
    return result;
}

//
QNArrayRef QNCipherGenerateMAC(QNArrayRef message, QNArrayRef token, QNArrayRef key) {
    return _sha256Contact(message, token, key);
}

static QN_STRONG QNArrayRef  _getIV(QNArrayRef token, QNArrayRef salt) {
    QNArrayRef data = QNArrayCreateCopy(token);
    if (salt) {
        QNArrayAppend(data, salt);
    }
    QNArrayRef result = QNArraySHA512(data);
    QNRelease(data);

    QNArrayRef iv = QNArraySlice(result, 0, 16);
    QNRelease(result);
    return iv;
}

//
static void QNCipherDeallocate(QNObjectRef object) {
    QNCipherRef cipher = (QNCipherRef)object;
    if (object) {
        QNRelease(cipher->privateKey);
        QNRelease(cipher->publicKey);
        QNRelease(cipher->saltA);
        QNRelease(cipher->saltB);
        QNRelease(cipher->ivSalt);
        QNDeallocate(cipher);
    }
}

QNMessageRef QNCipherEncryptMessage(QNCipherRef cipher, QNArrayRef plainData) {
    QNKeyRef publicKey = cipher->publicKey;
    QNArrayRef randomized = QNRandomVector(publicKey->length);
    QNArrayRef token = QNArrayPack(randomized);

    // derive keys
    QNArrayRef keyA = _sha256Contact(token, cipher->saltA, NULL);
    QNArrayRef keyB = _sha256Contact(token, cipher->saltB, NULL);

    // derive iv
    QNArrayRef iv = _getIV(token, cipher->ivSalt);

    // generate mac
    QNArrayRef mac = QNCipherGenerateMAC(plainData, token, keyB);

    QNArrayRef c0;
    QNArrayRef c1;

    cipher->asymmetricCipher->encrypt(publicKey, randomized, &c0, &c1);

    QNArrayRef pc = QNArrayCreateCopy(plainData);
    QNArrayAppend(pc, mac);

    QNArrayRef ciphered = cipher->symmetricCipher->encrypt(pc, keyA, iv);

    QNMessageRef message = QNMessageCreate(c0, c1, ciphered);

    QNRelease(randomized);
    QNRelease(token);
    QNRelease(keyA);
    QNRelease(keyB);
    QNRelease(iv);
    QNRelease(mac);
    QNRelease(c0);
    QNRelease(c1);
    QNRelease(pc);
    QNRelease(ciphered);

    return message;
}

QNArrayRef QNCipherDecryptMessage(QNCipherRef cipher, QNMessageRef message) {

    QNArrayRef rc_0 = message->c0;
    QNArrayRef rc_1 = message->c1;

    cipher->asymmetricCipher->decrypt(cipher->privateKey, rc_0, rc_1);
    QNArrayRef temp = rc_0;
    QNArrayRef decrypted_token = QNArrayPack(temp);

    // derive keys from data
    QNArrayRef decrypted_keyA = _sha256Contact(decrypted_token, cipher->saltA, NULL);
    QNArrayRef decrypted_keyB = _sha256Contact(decrypted_token, cipher->saltB, NULL);

    // derive iv
    QNArrayRef decrypted_iv = _getIV(decrypted_token, cipher->ivSalt);

    // decrypt ciphertext and derive mac
    QNArrayRef sem = cipher->symmetricCipher->decrypt(message->sym, decrypted_keyA, decrypted_iv);
    size_t count = sem->count;
    QNArrayRef decrypted_message = QNArraySlice(sem, 0, count - 32);
    QNArrayRef decrypted_mac = QNArraySlice(sem, count - 32, count);

    QNArrayRef receiver_mac = QNCipherGenerateMAC(decrypted_message, decrypted_token, decrypted_keyB);

    QNRelease(decrypted_keyA);
    QNRelease(decrypted_iv);
    QNRelease(decrypted_keyB);
    QNRelease(decrypted_token);
    QNRelease(sem);

    if (QNObjectEqual(receiver_mac, decrypted_mac)) {

        QNRelease(receiver_mac);
        QNRelease(decrypted_mac);

        return decrypted_message;
    } else {
        QNRelease(receiver_mac);
        QNRelease(decrypted_mac);
    }
    return NULL;
}
