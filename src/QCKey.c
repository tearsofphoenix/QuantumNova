//
// Created by Isaac on 2018/1/11.
//

#include "QCKey.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include <math.h>
#include <libtasn1.h>

static void QCKeyDeallocate(QCKeyRef key);
static QCKeyRef QCKeyCopy(QCKeyRef key);
static void QCKeyPrint(QCKeyRef key);
static bool QCKeyEqual(QCKeyRef key1, QCKeyRef key2);

static struct QCClass kQCKeyClass = {
        .name = "QCKey",
        .allocator = QCAllocator,
        .size = sizeof(struct QCKey),
        .deallocate = QCKeyDeallocate,
        .copy = QCKeyCopy,
        .print = QCKeyPrint,
        .equal = QCKeyEqual
};

QCKeyRef QCKeyCreateWith(QCArrayRef h0, QCArrayRef h1, QCArrayRef h1inv, QCArrayRef g, QCKeyConfig config) {
    QCKeyRef key = QCAllocate(&kQCKeyClass);
    key->h0 = QCRetain(h0);
    key->h1 = QCRetain(h1);
    key->h1inv = QCRetain(h1inv);
    key->g = QCRetain(g);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

static QCKeyRef QCKeyCopy(QCKeyRef key) {
    if (key) {
        QCKeyRef copy = key->isa->allocator(sizeof(*key));
        copy->length = key->length;
        copy->weight = key->weight;
        copy->error = key->error;
        copy->h0 = (QCArrayRef)QCObjectCopy(key->h0);
        copy->h1 = (QCArrayRef)QCObjectCopy(key->h1);
        copy->h1inv = (QCArrayRef)QCObjectCopy(key->h1inv);
        copy->g = (QCArrayRef)QCObjectCopy(key->g);
        return copy;
    }
    return NULL;
}

static void QCKeyDeallocate(QCKeyRef key) {
    if (key) {
        QCRelease(key->h0);
        QCRelease(key->h1);
        QCRelease(key->h1inv);
        QCRelease(key->g);
    }
}

static void QCKeyPrint(QCKeyRef key) {
    if (key) {
        if (key->g) {
            printf("<QCPublicKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        } else {
            printf("<QCPrivateKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        }
    }
}

static bool QCKeyEqual(QCKeyRef key1, QCKeyRef key2) {
    if (key1 && key2 && key1->isa == key2->isa) {
        return key1->length == key2->length
               && key1->weight == key2->weight
               && key1->error == key2->error
               && QCObjectEqual(key1->h0, key2->h0)
               && QCObjectEqual(key1->h1, key2->h1)
               && QCObjectEqual(key1->h1inv, key2->h1inv)
               && QCObjectEqual(key1->g, key2->g);
    }
    return false;
}

QCKeyRef QCKeyCreate(QCKeyConfig config, bool privateOrPublic) {
    QCKeyRef key = QCKeyCreateWith(NULL, NULL, NULL, NULL, config);
    return key;
}

void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey) {
    QCKeyRef privKey = QCKeyCreate(config, false);

    privKey->h0 = QCRandomWeightVector(config.length, config.weight);
    privKey->h1 = QCRandomWeightVector(config.length, config.weight);
    privKey->h1inv = QCArrayExpPoly(privKey->h1, (int64_t)pow(2, 1200) - 2);

    *privateKey = privKey;

    QCKeyRef pubKey = QCKeyCreate(config, true);
    pubKey->g = QCArrayMulPoly(privKey->h0, privKey->h1inv);

    *publicKey = pubKey;
}

QCKeyRef QCKeyCreateFromPEMFile(const char* filePath) {

}