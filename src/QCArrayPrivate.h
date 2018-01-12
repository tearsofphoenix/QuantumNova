//
// Created by Isaac on 2018/1/11.
//

#ifndef PQC_CRYPTO_QCARRAYPRIVATE_H
#define PQC_CRYPTO_QCARRAYPRIVATE_H


#include "QCArray.h"
#include <stdbool.h>
#include <ntsid.h>

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

extern void *_QCMallocData(QCArrayDataType type, int count, size_t  *size);

#define QCARRAYONE(array, iexp, dexp) do { \
                                            switch((array)->datatype) { \
                                                case QCDTInt: { \
                                                    int *d = (array)->data; \
                                                    iexp; \
                                                } \
                                                default: { \
                                                    double *d = (array)->data; \
                                                    dexp; \
                                                } \
                                            } \
                                       } while(0)

#define QCARRAYEACH(array, iexp, dexp) do { \
                                            int count = (array)->count; \
                                            switch((array)->datatype) { \
                                                case QCDTInt: { \
                                                    int *d = (array)->data; \
                                                    for(int i=0; i<count; ++i) { iexp; } \
                                                    break; \
                                                } \
                                                default: { \
                                                    double *d = (array)->data; \
                                                    for(int i=0; i<count; ++i) { dexp; } \
                                                    break; \
                                                } \
                                            } \
                                       } while(0)

extern void QCArrayFixConjugateHalf(QCArrayRef array);

extern void QCArrayXORAt(QCArrayRef array, int index, int value);

#endif //PQC_CRYPTO_QCARRAYPRIVATE_H
