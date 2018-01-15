//
// Created by Isaac on 2018/1/15.
//

#include "key-test.h"
#include "src/QCKey.h"

void key_test() {
    const char *path = "./aux/priv.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
}