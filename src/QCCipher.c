//
// Created by Isaac on 2018/1/11.
//

#include "QCCipher.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include "QCArrayPrivate.h"
#include <math.h>
#include <printf.h>


static double kH0ind[] = { 30,   49,  240,  250,  299,  376,  519,  602,  725,  746,  964,
                           1011, 1045, 1078, 1101, 1142, 1321, 1333, 1365, 1576, 1604, 1619,
                           1708, 1780, 2010, 2395, 2399, 2415, 2473, 2669, 2772, 3017, 3082,
                           3085, 3178, 3218, 3239, 3324, 3335, 3636, 3655, 3670, 4336, 4510,
                           4743};

static double kH1ind[] = {23,   55,  115,  194,  530,  571,  584,  621,  641,  693,  694,
                          805,  882,  910,  918,  934, 1010, 1185, 1201, 1554, 1565, 1890,
                          1898, 2006, 2073, 2152, 2361, 2382, 2651, 2938, 3370, 3419, 3505,
                          3571, 3878, 4033, 4160, 4228, 4283, 4511, 4560, 4651, 4657, 4683,
                          4695};

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

    QCArrayFree(t2);

    return temp;
}

typedef struct {
    QCArrayRef synd;
    QCArrayRef array;
    int kBL;
} QCLoopContext;

static void _h0LoopFunc(double num, int index, const void *ctx) {
    QCLoopContext *c = ctx;
    QCArrayRef synd = c->synd;
    int kBL = c->kBL;
    QCArrayRef array = c->array;
    for (int j = 0; j < synd->count; ++ j) {
        if (QCArrayValueAt(synd, j)) {
            int idx = (j + kBL - (int)num) % kBL;
            QCArrayAddAt(array, idx, 1);
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

static void _blockLoopFunc(double dj, int index, const void *ctx) {
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
    QCArrayRef unsat_H0 = QCArrayCreate(kBL);

    QCLoopContext ctx;
    ctx.synd = synd;
    ctx.array = unsat_H0;
    ctx.kBL = kBL;
    QCArrayForeach(H0_ind, _h0LoopFunc, &ctx);

    QCArrayRef unsat_H1 = QCArrayCreate(kBL);
    ctx.array = unsat_H1;
    QCArrayForeach(H1_ind, _h0LoopFunc, &ctx);

    int rounds = 10;
    int delta = 5;
    int threshold = 100;
    int r = 0;

    QCArrayCompareRaw(H0_ind, kH0ind);
    QCArrayCompareRaw(H1_ind, kH1ind);

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

        QCArrayFree(round_unsat_H0);
        QCArrayFree(round_unsat_H1);
    }

    QCArrayFree(unsat_H0);
    QCArrayFree(unsat_H1);
    QCArrayFree(H0_ind);
    QCArrayFree(H1_ind);
    QCArrayFree(synd);

    return c0;
}
