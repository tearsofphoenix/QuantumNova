//
// Created by Isaac on 2018/1/15.
//

#include <printf.h>
#include <src/QCKeyPrivate.h>
#include "key-test.h"
#include "data.h"
#include "src/QCKey.h"
#include "src/QCArrayPrivate.h"
#include "src/QCMessagePrivate.h"

static void private_key_test() {
    printf("----------private key test start---------\n");
    const char *path = "./aux/priv.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
    if(QCArrayCompareRaw(key->h0, H0, QCDTDouble)) {
        printf("h0 passed\n");
    }
    if(QCArrayCompareRaw(key->h1, H1, QCDTDouble)) {
        printf("h1 passed\n");
    }
    if(QCArrayCompareRaw(key->h1inv, H1_inv, QCDTDouble)) {
        printf("h1inv passed\n");
    }
}

static void public_key_test() {
    printf("----------public key test start---------\n");
    const char *path = "./aux/pub.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
    if(QCArrayCompareRaw(key->g, G, QCDTDouble)) {
        printf("G passed\n");
    }
}

static void message_test() {
    printf("----------message test start---------\n");
    const char *path = "./aux/enc.data";
    QCMessageRef message = QCMessageCreateFromPEMFile(path);
    if(QCArrayCompareRaw(message->c0, C0, QCDTDouble)) {
        printf("C0 passed\n");
    }
    if(QCArrayCompareRaw(message->c1, C1, QCDTDouble)) {
        printf("C1 passed\n");
    }
}

void key_test() {
    private_key_test();

    public_key_test();
}