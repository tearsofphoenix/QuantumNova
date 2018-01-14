//
// Created by Isaac on 2018/1/13.
//

#include <printf.h>
#include <memory.h>
#include "array-test.h"
#include "src/QCArray.h"
#include "src/vendor/sha256.h"
#include "fft-test.h"

void array_test() {

    QCByte text1[] = {"abc"};
    QCByte text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};

    QCByte hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
                                     0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
    QCByte hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
                                     0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};

    QCArrayRef a1 = QCArrayCreateWithByte(text1, strlen(text1), false);
    QCArrayRef s1 = QCArraySHA256(a1);
    QCObjectPrint(s1);
    if (QCArrayCompareRaw(s1, hash1, QCDTByte)) {
        printf("equal\n");
    }

    QCArrayRef a2 = QCArrayCreateWithByte(text2, strlen(text2), false);
    QCArrayRef s2 = QCArraySHA256(a2);
    QCObjectPrint(s2);
    if (QCArrayCompareRaw(s2, hash2, QCDTByte)) {
        printf("equal\n");
    }

    QCRelease(a1);
    QCRelease(a2);

    QCRelease(s1);
    QCRelease(s2);
}
