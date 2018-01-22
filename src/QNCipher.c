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

void QNCipherEncrypt(QNCipherRef cipher, QNArrayRef random, QNArrayRef *u, QNArrayRef *v) {
// non-constant weight to achieve cipertext indistinguishability
    QNKeyRef publicKey = cipher->publicKey;
    QNArrayRef temp = QNArrayMulPoly(publicKey->g, random);

    QNArrayRef t = QNRandomWeightVector(publicKey->length, publicKey->error + QNRandomFlipCoin());
    QNArrayAddArray(temp, t);
    QNArrayMod(temp, 2);

    QNRelease(t);

    QNArrayRef t2 = QNRandomWeightVector(publicKey->length, publicKey->error + QNRandomFlipCoin());
    QNArrayRef copy = QNArrayCreateCopy(random);
    QNArrayAddArray(copy, t2);
    QNArrayMod(copy, 2);

    QNRelease(t2);

    *u = copy;
    *v = temp;
}

QNArrayRef QNCipherSyndrome(QNCipherRef cipher, QNArrayRef c0, QNArrayRef c1) {
    QNKeyRef privateKey = cipher->privateKey;
    QNArrayRef temp = QNArrayMulPoly(privateKey->h0, c0);
    QNArrayRef t2 = QNArrayMulPoly(privateKey->h1, c1);

    QNArrayAddArray(temp, t2);

    QNArrayMod(temp, 2);

    QNRelease(t2);

    return temp;
}

typedef struct {
    QNArrayRef synd;
    QNArrayRef array;
    size_t kBL;
} QNLoopContext;

static void _h0LoopFunc(int num, int index, const void *ctx) {
    QNLoopContext *c = ctx;
    QNArrayRef synd = c->synd;
    size_t kBL = c->kBL;
    QNArrayRef array = c->array;
    int *d = array->data;
    for (int j = 0; j < synd->count; ++ j) {
        if ((int)QNArrayValueAt(synd, j) != 0) {
            size_t idx = (j + kBL - num) % kBL;
            d[idx] += 1;
        }
    }
}

typedef struct {
    QNArrayRef synd;
    QNArrayRef array0;
    QNArrayRef array1;
    QNArrayRef un0;
    QNArrayRef un1;
    int i;
    size_t kBL;
} QNBlockLoopContext;

static void _blockLoopFunc(int dj, int index, const void *ctx) {
    int j = dj;
    QNBlockLoopContext *c = ctx;

    QNArrayRef synd = c->synd;
    QNArrayRef array0 = c->array0;
    QNArrayRef array1 = c->array1;
    QNArrayRef un0 = c->un0;
    QNArrayRef un1 = c->un1;
    int i = c->i;
    size_t kBL = c->kBL;

    bool increase = QNArrayValueAt(synd, (i + j) % kBL) == 0;

    for (int _ = 0; _ < array0->count; ++_) {
        int k = (int)QNArrayValueAt(array0, _);
        size_t m = (i + j - k + kBL) % kBL;
        if (increase) {
            QNArrayAddAt(un0, m, 1);
        } else {
            QNArrayAddAt(un0, m, -1);
        }
    }

    for (int _ = 0; _ < array1->count; ++_) {
        int k = (int)QNArrayValueAt(array1, _);
        size_t m = (i + j - k + kBL) % kBL;
        if (increase) {
            QNArrayAddAt(un1, m, 1);
        } else {
            QNArrayAddAt(un1, m, -1);
        }
    }

    size_t idx = (i + j) % kBL;
    QNArrayXORAt(synd, idx, 1);
}

void QNCipherDecrypt(QNCipherRef cipher, QNArrayRef c0, QNArrayRef c1) {
    QNKeyRef privateKey = cipher->privateKey;
    QNArrayRef synd = QNCipherSyndrome(cipher, c0, c1);
    // compute correlations with syndrome
    QNArrayRef H0_ind = QNArrayGetNoZeroIndices(privateKey->h0);
    QNArrayRef H1_ind = QNArrayGetNoZeroIndices(privateKey->h1);

    size_t kBL = privateKey->length;
    QNArrayRef unsat_H0 = QNArrayCreateWithInt(NULL, kBL, true);

    QNLoopContext ctx;
    ctx.synd = synd;
    ctx.array = unsat_H0;
    ctx.kBL = kBL;
    QNArrayForeach(H0_ind, _h0LoopFunc, &ctx);

    QNArrayRef unsat_H1 = QNArrayCreateWithInt(NULL, kBL, true);
    ctx.array = unsat_H1;
    QNArrayForeach(H1_ind, _h0LoopFunc, &ctx);

    int rounds = 10;
    int delta = 5;
    int threshold = 100;
    int r = 0;

    QNArrayRef round_unsat_H0 = NULL;
    QNArrayRef round_unsat_H1 = NULL;
    while (true) {
        int max_unsat = (int)fmax(QNArrayMax(unsat_H0), QNArrayMax(unsat_H1));

        // if so, we are done decoding
        if (max_unsat == 0) {
            break;
        }

        // we have reach the upper bound on rounds
        if (r >= rounds) {
            printf("Decryption error.\n");
            break;
        }
        r += 1;

        // update threshold
        if (max_unsat > delta) {
            threshold = max_unsat - delta;
        }

        round_unsat_H0 = QNArrayCreateCopy(unsat_H0);
        round_unsat_H1 = QNArrayCreateCopy(unsat_H1);

        // first block sweep
        for (int i = 0; i < kBL; ++i) {
            if (QNArrayValueAt(round_unsat_H0, i) <= threshold) {
                continue;
            }

            QNBlockLoopContext ctx;
            ctx.synd = synd;
            ctx.kBL = kBL;
            ctx.i = i;
            ctx.array0 = H0_ind;
            ctx.array1 = H1_ind;
            ctx.un0 = unsat_H0;
            ctx.un1 = unsat_H1;

            QNArrayForeach(H0_ind, _blockLoopFunc, &ctx);

            QNArrayXORAt(c0, i, 1);
        }

        // second block sweep
        for(int i = 0; i < kBL; ++i) {
            if (QNArrayValueAt(round_unsat_H1, i) <= threshold) {
                continue;
            }

            QNBlockLoopContext ctx;
            ctx.synd = synd;
            ctx.kBL = kBL;
            ctx.i = i;
            ctx.array0 = H0_ind;
            ctx.array1 = H1_ind;
            ctx.un0 = unsat_H0;
            ctx.un1 = unsat_H1;

            QNArrayForeach(H1_ind, _blockLoopFunc, &ctx);

            QNArrayXORAt(c1, i, 1);
        }

        QNRelease(round_unsat_H0);
        QNRelease(round_unsat_H1);
        round_unsat_H0 = NULL;
        round_unsat_H1 = NULL;
    }

    QNRelease(round_unsat_H0);
    QNRelease(round_unsat_H1);

    QNRelease(unsat_H0);
    QNRelease(unsat_H1);
    QNRelease(H0_ind);
    QNRelease(H1_ind);
    QNRelease(synd);

    return; // c0
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
QN_STRONG QNArrayRef QNCipherGenerateMAC(QNArrayRef message, QNArrayRef token, QNArrayRef key) {
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
    QNCipherEncrypt(cipher, randomized, &c0, &c1);

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

    QNCipherDecrypt(cipher, rc_0, rc_1);
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
