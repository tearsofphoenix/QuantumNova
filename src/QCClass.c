//
// Created by Isaac on 2018/1/12.
//

#include "QCClass.h"
#include "QCObjectPrivate.h"
#include <fftw3.h>
#include <memory.h>

static struct QCClass kBaseClass = {
        .name = "QCObject",
        .allocator = QCAllocator,
        .retain = QCRetain,
        .release = QCRelease,
        .copy = NULL,
        .deallocate = QCDeallocate,
};

const QCClassRef kQCBaseClassRef = &kBaseClass;

void *QCAllocator(size_t size) {
    void *mem = fftw_malloc(size);
    memset(mem, 0, size);
    return mem;
}

void QCDeallocate(const void *p) {
    fftw_free(p);
}
