//
// Created by Isaac on 2018/1/15.
//

#include <memory.h>
#include "fft-test.h"
#include "cipher-test.h"
#include "array-test.h"
#include "key-test.h"
#include <printf.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

static int foo() {

    printf("%sred\n", KRED);
    printf("%sgreen\n", KGRN);
    printf("%syellow\n", KYEL);
    printf("%sblue\n", KBLU);
    printf("%smagenta\n", KMAG);
    printf("%scyan\n", KCYN);
    printf("%swhite\n", KWHT);
    printf("%snormal\n", KNRM);

    return 0;
}

int main() {

    fft_test_all(1);

    cipher_test();

    array_test();

    key_test();

    return 0;
}