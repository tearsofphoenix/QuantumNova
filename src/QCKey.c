//
// Created by Isaac on 2018/1/11.
//

#include "QCKey.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include <math.h>
#include <libtasn1.h>

QCKeyRef QCKeyCreate(int length, int weight, bool privateOrPublic) {

}

void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey) {
    QCKeyRef privKey = QCKeyCreate(config.length, config.weight, false);
    privKey->length = config.length;
    privKey->weight = config.weight;
    privKey->error = config.error;
    privKey->h0 = QCRandomWeightVector(config.length, config.weight);
    privKey->h1 = QCRandomWeightVector(config.length, config.weight);
    privKey->h1inv = QCArrayExpPoly(privKey->h1, (int64_t)pow(2, 1200) - 2);

    *privateKey = privKey;

    QCKeyRef pubKey = QCKeyCreate(config.length, config.weight, true);
    pubKey->g = QCArrayMulPoly(privKey->h0, privKey->h1inv);
    pubKey->length = config.length;
    pubKey->weight = config.weight;
    pubKey->error = config.error;

    *publicKey = pubKey;
}

QCKeyRef QCKeyCreateFromPEMFile(const char* filePath) {

}