//
// Created by Isaac on 2018/1/15.
//

#include <printf.h>
#include <src/QCKeyPrivate.h>
#include "key-test.h"
#include "data.h"
#include "src/QCKey.h"
#include "src/QCArrayPrivate.h"

void key_test() {
    printf("----------key test start---------");
    const char *path = "./aux/priv.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
    if(QCArrayCompareRaw(key->h0, H0, QCDTDouble)) {
        printf("h0 passed");
    }
    if(QCArrayCompareRaw(key->h1, H1, QCDTDouble)) {
        printf("h1 passed");
    }
    if(QCArrayCompareRaw(key->h1inv, H1_inv, QCDTDouble)) {
        printf("h1inv passed");
    }
}