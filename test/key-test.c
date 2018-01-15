//
// Created by Isaac on 2018/1/15.
//

#include <printf.h>
#include "key-test.h"
#include "src/QCKey.h"

void key_test() {
    printf("----------key test start---------");
    const char *path = "./aux/priv.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
}