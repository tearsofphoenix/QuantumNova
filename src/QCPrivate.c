//
// Created by Isaac on 2018/1/11.
//

#include "QCPrivate.h"
#include <fftw3.h>

void *_QCMalloc(size_t size) {
    return fftw_malloc(size);
}

void _QCFree(void *pointer) {
    return fftw_free(pointer);
}