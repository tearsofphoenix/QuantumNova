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
        .deallocate = QCDeallocate,
};

const QCClassRef kQCBaseClassRef = &kBaseClass;

const void *QCAllocator(size_t size) {
    QCObjectRef object = fftw_malloc(size);
    object->retainCount = 1;
    return object;
}

void QCDeallocate(const void *p) {
    fftw_free(p);
}
