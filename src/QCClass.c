//
// Created by Isaac on 2018/1/12.
//

#include "QCClass.h"
#include "QCObjectPrivate.h"
#include <fftw3.h>

static struct QCClass kBaseClass = {
        .name = "QCObject",
        .allocator = QCAllocator,
        .retain = QCRetain,
        .release = QCRelease,
        .copy = NULL,
        .deallocate = QCDeallocate,
};

const QCClassRef kQCBaseClassRef = &kBaseClass;

const void *QCAllocator(size_t size) {
    return fftw_malloc(size);
}

void QCDeallocate(const void *p) {
    fftw_free(p);
}
