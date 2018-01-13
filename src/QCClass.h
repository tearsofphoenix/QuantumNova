//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCCLASS_H
#define PQC_CRYPTO_QCCLASS_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned char QCByte;
typedef unsigned int QCWord;

typedef struct QCClass QCClass;
typedef QCClass *QCClassRef;

typedef const void * (* QCAllocatorFunc)(size_t);
typedef void (* QCFreeFunc)(const void *);
typedef const void *(* QCCopyFunc)(const void *);
typedef const void *(* QCRetainFunc)(const void *);
typedef void (* QCReleaseFunc)(const void *);
typedef void (* QCPrintFunc)(const void *);
typedef bool (* QCEqualFunc)(const void *, const void *);

#ifndef QCCLASSFIELDS

#define QCCLASSFIELDS const QCClassRef base; \
                      const char *name; \
                      size_t size; \
                      QCAllocatorFunc allocator; \
                      QCFreeFunc deallocate; \
                      QCCopyFunc copy; \
                      QCRetainFunc retain; \
                      QCReleaseFunc release; \
                      QCPrintFunc print; \
                      QCEqualFunc equal;

#endif

struct QCClass {
    QCCLASSFIELDS
};


extern const QCClassRef kQCBaseClassRef;

extern const void *QCAllocator(size_t size);

extern void QCDeallocate(const void *p);

#endif //PQC_CRYPTO_QCCLASS_H
