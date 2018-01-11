//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAYPRIVATE_H
#define PQC_CRYPTO_QCARRAYPRIVATE_H

#include <stdbool.h>
#include "QCArray.h"

struct QCArray {
    void *data;
    int count; // count of number in data
    struct {
        unsigned int fft: 1; // data contains fft result
        unsigned int needfree: 1; // if data need to be freed
        unsigned int datatype: 3; // data type
        unsigned int unused: 27;
    };
};

#endif //PQC_CRYPTO_QCARRAYPRIVATE_H
