//
// Created by Isaac on 2018/1/22.
//

#include "QNAsymmetricCipher.h"
#include "QNKeyPrivate.h"
#include "QNRandom.h"
#include "QNArrayPrivate.h"
#include <math.h>

static void _encrypt(QNKeyRef publicKey, QNArrayRef random, QNArrayRef *u, QNArrayRef *v) {
// non-constant weight to achieve cipertext indistinguishability
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

static QNArrayRef _syndrome(QNKeyRef privateKey, QNArrayRef c0, QNArrayRef c1) {
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
    const QNLoopContext *c = ctx;
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
    const QNBlockLoopContext *c = ctx;

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

static void _decrypt(QNKeyRef privateKey, QNArrayRef c0, QNArrayRef c1) {
    QNArrayRef synd = _syndrome(privateKey, c0, c1);
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

static struct QNAsymmetricCipher kCipher = {
        .encrypt = _encrypt,
        .decrypt = _decrypt,
        .syndrome = _syndrome
};

QNAsymmetricCipherRef QNGetAsymmetricCipher() {
    return &kCipher;
}
