//
// Created by Isaac on 2018/1/11.
//

#include "QCCipher.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include "QCObject.h"
#include "QCArrayPrivate.h"
#include <math.h>
#include <printf.h>

void QCCipherEncrypt(QCKeyRef publicKey, QCArrayRef message, QCArrayRef *u, QCArrayRef *v) {
// non-constant weight to achieve cipertext indistinguishability

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

QCArrayRef QCCipherSyndrome(const QCKeyRef privateKey, QCArrayRef c0, QCArrayRef c1) {
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
    int kBL;
} QCLoopContext;

static void _h0LoopFunc(int num, int index, const void *ctx) {
    QCLoopContext *c = ctx;
    QCArrayRef synd = c->synd;
    int kBL = c->kBL;
    QCArrayRef array = c->array;
    int *d = array->data;
    for (int j = 0; j < synd->count; ++ j) {
        if ((int)QCArrayValueAt(synd, j) != 0) {
            int idx = (j + kBL - num) % kBL;
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
    int kBL;
} QCBlockLoopContext;

static void _blockLoopFunc(int dj, int index, const void *ctx) {
    int j = (int)dj;
    QCBlockLoopContext *c = ctx;

    QCArrayRef synd = c->synd;
    QCArrayRef array0 = c->array0;
    QCArrayRef array1 = c->array1;
    QCArrayRef un0 = c->un0;
    QCArrayRef un1 = c->un1;
    int i = c->i;
    int kBL = c->kBL;

    bool increase = QCArrayValueAt(synd, (i + j) % kBL) == 0;

    for (int _ = 0; _ < array0->count; ++i) {
        int k = (int)QCArrayValueAt(array0, _);
        int m = (i + j - k + kBL) % kBL;
        if (increase) {
            QCArrayAddAt(un0, m, 1);
        } else {
            QCArrayAddAt(un0, m, -1);
        }
    }

    for (int _ = 0; _ < array1->count; ++i) {
        int k = (int)QCArrayValueAt(array1, _);
        int m = (i + j - k + kBL) % kBL;
        if (increase) {
            QCArrayAddAt(un1, m, 1);
        } else {
            QCArrayAddAt(un1, m, -1);
        }
    }

    int idx = (i + j) % kBL;
    QCArrayXORAt(synd, idx, 1);
}

QCArrayRef QCCipherDecrypt(const QCKeyRef privateKey, QCArrayRef c0, QCArrayRef c1) {

    QCArrayRef synd = QCCipherSyndrome(privateKey, c0, c1);
    // compute correlations with syndrome
    QCArrayRef H0_ind = QCArrayGetNoZeroIndices(privateKey->h0);
    QCArrayRef H1_ind = QCArrayGetNoZeroIndices(privateKey->h1);

    int kBL = privateKey->length;
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

    printf("--------index ok---------\n");

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
