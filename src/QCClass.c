//
// Created by Isaac on 2018/1/12.
//

#include "QCClass.h"
#include "QCObject.h"
#include <fftw3.h>

static struct QCClass kBaseClass = {
        .name = "QCObject",
        .allocator = QCAllocator,
        .retain = QCRetain,
        .release = QCRelease,
        .copy = NULL,
        .deallocate = QCFree,
};

const QCClassRef kQCBaseClassRef = &kBaseClass;

const void *QCAllocator(size_t size) {
    return fftw_malloc(size);
}

void QCFree(const void *p) {
    fftw_free(p);
}
