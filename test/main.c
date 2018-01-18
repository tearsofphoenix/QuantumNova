//
// Created by Isaac on 2018/1/15.
//

#include <memory.h>
#include "fft-test.h"
#include "cipher-test.h"
#include "array-test.h"
#include "key-test.h"
#include "src/QCObjectPrivate.h"

int main() {

    _QCStartMemoryRecord();

    fft_test_all();

//    cipher_test();

//    array_test();

//    key_test();


    return 0;
}