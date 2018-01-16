//
// Created by Isaac on 2018/1/11.
//

#include "QCCipher.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include "QCArrayPrivate.h"
#include "QCCipherPrivate.h"
#include "QCMessagePrivate.h"
#include "vendor/sha256.h"
#include "vendor/aes.h"
#include <math.h>
#include <printf.h>
#include <memory.h>

static QCByte kSaltA[] = {"this is just a salt"};
static QCByte kSaltB[] = {"this is another a salt"};
static QCByte kIVSalt[] = {"third salt"};

static void QCCipherDeallocate(QCObjectRef object);

static struct QCClass kQCCipherClass = {
        .name = "QCCipher",
        .size = sizeof(struct QCCipher),
        .allocator = QCAllocator,
        .deallocate = QCCipherDeallocate
};

QCCipherRef QCCipherCreate(void) {
    QCCipherRef cipher = QCAllocate(&kQCCipherClass);
    cipher->saltA = QCArrayCreateWithByte(kSaltA, strlen(kSaltA), false);
    cipher->saltB = QCArrayCreateWithByte(kSaltB, strlen(kSaltB), false);
    cipher->ivSalt = QCArrayCreateWithByte(kIVSalt, strlen(kIVSalt), false);
    return cipher;
}

QCKeyRef QCCipherGetPrivateKey(QCCipherRef cipher) {
    return cipher->privateKey;
}

QCKeyRef QCCipherGetPublicKey(QCCipherRef cipher) {
    return cipher->publicKey;
}

void QCCipherSetPrivateKey(QCCipherRef cipher, QCKeyRef privateKey) {
    QCRelease(cipher->privateKey);
    cipher->privateKey = QCRetain(privateKey);
}

void QCCipherSetPublicKey(QCCipherRef cipher, QCKeyRef publicKey) {
    QCRelease(cipher->publicKey);
    cipher->publicKey = QCRetain(publicKey);
}


void QCCipherEncrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef *u, QCArrayRef *v) {
// non-constant weight to achieve cipertext indistinguishability
    QCKeyRef publicKey = cipher->publicKey;
    QCArrayRef temp = QCArrayMulPoly(publicKey->g, message);

    QCArrayRef t = QCRandomWeightVector(publicKey->length, publicKey->error + QCRandomFlipCoin());
    QCArrayAddArray(temp, t);
    QCArrayMod(temp, 2);

    QCArrayRef t2 = QCRandomWeightVector(publicKey->length, publicKey->error + QCRandomFlipCoin());
    QCArrayRef copy = QCArrayCreateCopy(message);
    QCArrayAddArray(copy, t2);
    QCArrayMod(copy, 2);

    *u = temp;
    *v = copy;
}

QCArrayRef QCCipherSyndrome(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1) {
    QCKeyRef privateKey = cipher->privateKey;
    QCArrayRef temp = QCArrayMulPoly(privateKey->h0, c0);
    QCArrayRef t2 = QCArrayMulPoly(privateKey->h1, c1);

    QCArrayAddArray(temp, t2);

    QCArrayMod(temp, 2);

    QCRelease(t2);

    return temp;
}

typedef struct {
    QCArrayRef synd;
    QCArrayRef array;
    size_t kBL;
} QCLoopContext;

static void _h0LoopFunc(int num, int index, const void *ctx) {
    QCLoopContext *c = ctx;
    QCArrayRef synd = c->synd;
    size_t kBL = c->kBL;
    QCArrayRef array = c->array;
    int *d = array->data;
    for (int j = 0; j < synd->count; ++ j) {
        if ((int)QCArrayValueAt(synd, j) != 0) {
            size_t idx = (j + kBL - num) % kBL;
            d[idx] += 1;
        }
    }
}

typedef struct {
    QCArrayRef synd;
    QCArrayRef array0;
    QCArrayRef array1;
    QCArrayRef un0;
    QCArrayRef un1;
    int i;
    size_t kBL;
} QCBlockLoopContext;

static void _blockLoopFunc(int dj, int index, const void *ctx) {
    int j = dj;
    QCBlockLoopContext *c = ctx;

    QCArrayRef synd = c->synd;
    QCArrayRef array0 = c->array0;
    QCArrayRef array1 = c->array1;
    QCArrayRef un0 = c->un0;
    QCArrayRef un1 = c->un1;
    int i = c->i;
    size_t kBL = c->kBL;

    bool increase = QCArrayValueAt(synd, (i + j) % kBL) == 0;

    for (int _ = 0; _ < array0->count; ++_) {
        int k = (int)QCArrayValueAt(array0, _);
        size_t m = (i + j - k + kBL) % kBL;
        if (increase) {
            QCArrayAddAt(un0, m, 1);
        } else {
            QCArrayAddAt(un0, m, -1);
        }
    }

    for (int _ = 0; _ < array1->count; ++_) {
        int k = (int)QCArrayValueAt(array1, _);
        size_t m = (i + j - k + kBL) % kBL;
        if (increase) {
            QCArrayAddAt(un1, m, 1);
        } else {
            QCArrayAddAt(un1, m, -1);
        }
    }

    size_t idx = (i + j) % kBL;
    QCArrayXORAt(synd, idx, 1);
}

QCArrayRef QCCipherDecrypt(QCCipherRef cipher, QCArrayRef c0, QCArrayRef c1) {
    QCKeyRef privateKey = cipher->privateKey;
    QCArrayRef synd = QCCipherSyndrome(cipher, c0, c1);
    // compute correlations with syndrome
    QCArrayRef H0_ind = QCArrayGetNoZeroIndices(privateKey->h0);
    QCArrayRef H1_ind = QCArrayGetNoZeroIndices(privateKey->h1);

    size_t kBL = privateKey->length;
    QCArrayRef unsat_H0 = QCArrayCreateWithInt(NULL, kBL, false);

    QCLoopContext ctx;
    ctx.synd = synd;
    ctx.array = unsat_H0;
    ctx.kBL = kBL;
    QCArrayForeach(H0_ind, _h0LoopFunc, &ctx);

    QCArrayRef unsat_H1 = QCArrayCreateWithInt(NULL, kBL, false);
    ctx.array = unsat_H1;
    QCArrayForeach(H1_ind, _h0LoopFunc, &ctx);

    int rounds = 10;
    int delta = 5;
    int threshold = 100;
    int r = 0;

    while (true) {
        int max_unsat = (int)fmax(QCArrayMax(unsat_H0), QCArrayMax(unsat_H1));

        // if so, we are done decoding
        if (max_unsat == 0) {
            break;
        }

        // we have reach the upper bound on rounds
        if (r >= rounds) {
            printf("Decryption error");
            break;
        }
        r += 1;

        // update threshold
        if (max_unsat > delta) {
            threshold = max_unsat - delta;
        }
        QCArrayRef round_unsat_H0 = QCArrayCreateCopy(unsat_H0);
        QCArrayRef round_unsat_H1 = QCArrayCreateCopy(unsat_H1);

        // first block sweep
        for (int i = 0; i < kBL; ++i) {
            if (QCArrayValueAt(round_unsat_H0, i) <= threshold) {
                continue;
            }

            QCBlockLoopContext ctx;
            ctx.synd = synd;
            ctx.kBL = kBL;
            ctx.i = i;
            ctx.array0 = H0_ind;
            ctx.array1 = H1_ind;
            ctx.un0 = unsat_H0;
            ctx.un1 = unsat_H1;

            QCArrayForeach(H0_ind, _blockLoopFunc, &ctx);

            QCArrayXORAt(c0, i, 1);
        }

        // second block sweep
        for(int i = 0; i < kBL; ++i) {
            if (QCArrayValueAt(round_unsat_H1, i) <= threshold) {
                continue;
            }

            QCBlockLoopContext ctx;
            ctx.synd = synd;
            ctx.kBL = kBL;
            ctx.i = i;
            ctx.array0 = H0_ind;
            ctx.array1 = H1_ind;
            ctx.un0 = unsat_H0;
            ctx.un1 = unsat_H1;

            QCArrayForeach(H1_ind, _blockLoopFunc, &ctx);

            QCArrayXORAt(c1, i, 1);
        }

        QCRelease(round_unsat_H0);
        QCRelease(round_unsat_H1);
    }

    QCRelease(unsat_H0);
    QCRelease(unsat_H1);
    QCRelease(H0_ind);
    QCRelease(H1_ind);
    QCRelease(synd);

    return c0;
}

//
QCArrayRef QCCipherGenerateMAC(QCArrayRef message, QCArrayRef token, QCArrayRef key) {
    QCArrayRef data = QCArrayCreateCopy(message);
    QCArrayAppend(data, token);
    QCArrayAppend(data, key);
    QCArrayRef result = QCArraySHA256(data);
    QCRelease(data);
    return result;
}

//
static void QCCipherDeallocate(QCObjectRef object) {
    QCCipherRef cipher = (QCCipherRef)object;
    if (object) {
        QCRelease(cipher->privateKey);
        QCRelease(cipher->publicKey);
    }
}

QCArrayRef QCCipherSymmetricEncrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef key, QCArrayRef iv) {
    size_t messageSize = message->count;
    BYTE *out = cipher->isa->allocator(messageSize * sizeof(QCByte));
    WORD key_schedule[60];
    int keysize = 256;
    aes_key_setup(key->data, key_schedule, keysize);

    aes_encrypt_cbc(message->data, messageSize * sizeof(QCByte), out, key_schedule, keysize, iv->data);
    QCArrayRef array = QCArrayCreateWithByte(out, messageSize, false);
    array->needfree = true;
    return array;
}

QCArrayRef QCCipherSymmetricDecrypt(QCCipherRef cipher, QCArrayRef message, QCArrayRef key, QCArrayRef iv) {
    size_t messageSize = message->count;
    BYTE *out = cipher->isa->allocator(messageSize * sizeof(QCByte));
    WORD key_schedule[60];
    int keysize = 256;
    aes_key_setup(key->data, key_schedule, keysize);

    aes_decrypt_cbc(message->data, messageSize * sizeof(QCByte), out, key_schedule, keysize, iv->data);
    QCArrayRef array = QCArrayCreateWithByte(out, messageSize, false);
    array->needfree = true;
    QCArrayRef ref = QCArrayPKCS7Decode(array);
    QCRelease(array);
    return ref;
}

QCMessageRef QCCipherEncryptMessage(QCCipherRef cipher, QCArrayRef plainData) {
    QCKeyRef publicKey = cipher->publicKey;
    QCArrayRef randomized = QCRandomVector(publicKey->length);
    QCArrayRef token = QCArrayPack(randomized);

    // derive keys
    QCArrayRef temp = QCArrayCreateCopy(token);
    QCArrayAppend(temp, cipher->saltA);
    QCArrayRef keyA = QCArraySHA256(temp);
    QCRelease(temp);

    temp = QCArrayCreateCopy(token);
    QCArrayAppend(temp, cipher->saltB);
    QCArrayRef keyB = QCArraySHA256(temp);
    QCRelease(temp);

    // derive iv
    temp = QCArrayCreateCopy(token);
    QCArrayAppend(temp, cipher->ivSalt);
    QCArrayRef tem = QCArraySHA512(temp);
    QCArrayRef iv = QCArraySlice(tem, 0, 16);
    QCRelease(temp);
    QCRelease(tem);

    // generate mac
    QCArrayRef mac = QCCipherGenerateMAC(plainData, token, keyB);

    QCArrayRef c0;
    QCArrayRef c1;
    QCCipherEncrypt(cipher, randomized, &c0, &c1);

    QCArrayRef ciphered = QCCipherSymmetricEncrypt(cipher, plainData, keyA, iv);

    QCMessageRef message = QCMessageCreate(c0, c1, ciphered);

    QCRelease(ciphered);
    QCRelease(c0);
    QCRelease(c1);
    QCRelease(mac);
    QCRelease(randomized);

    return message;
}

QCArrayRef QCCipherDecryptMessage(QCCipherRef cipher, QCMessageRef message) {

    QCArrayRef rc_0 = message->c0;
    QCArrayRef rc_1 = message->c1;

    QCArrayRef temp = QCCipherDecrypt(cipher, rc_0, rc_1);
    QCArrayRef decrypted_token = QCArrayPack(temp);

    QCRelease(temp);

    // derive keys from data
    QCArrayRef dt_copy = QCArrayCreateCopy(decrypted_token);
    QCArrayAppend(dt_copy, cipher->saltA);
    QCArrayRef decrypted_keyA = QCArraySHA256(dt_copy);
    QCRelease(dt_copy);

    dt_copy = QCArrayCreateCopy(decrypted_token);
    QCArrayAppend(dt_copy, cipher->saltB);
    QCArrayRef decrypted_keyB = QCArraySHA256(dt_copy);
    QCRelease(dt_copy);

    // derive iv
    dt_copy = QCArrayCreateCopy(decrypted_token);
    QCArrayAppend(dt_copy, cipher->ivSalt);
    QCArrayRef tem = QCArraySHA512(dt_copy);
    QCRelease(dt_copy);
    QCArrayRef decrypted_iv = QCArraySlice(tem, 0, 16);
    QCRelease(tem);

    // decrypt ciphertext and derive mac
    QCArrayRef sem = QCCipherSymmetricDecrypt(cipher, message->sym, decrypted_keyA, decrypted_iv);
    size_t count = sem->count;
    QCArrayRef decrypted_message = QCArraySlice(sem, 0, count - 32);
    QCArrayRef decrypted_mac = QCArraySlice(sem, count - 32, count);

    QCArrayRef receiver_mac = QCCipherGenerateMAC(decrypted_message, decrypted_token, decrypted_keyB);

    QCRelease(decrypted_keyA);
    QCRelease(decrypted_iv);
    QCRelease(decrypted_keyB);

    if (QCObjectEqual(receiver_mac, decrypted_mac)) {

        QCRelease(receiver_mac);
        QCRelease(decrypted_mac);

        return decrypted_message;
    } else {
        QCRelease(receiver_mac);
        QCRelease(decrypted_mac);
    }
    return NULL;
}
