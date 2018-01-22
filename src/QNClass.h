//
// Created by Isaac on 2018/1/12.
//

#ifndef QN_QNCLASS_H
#define QN_QNCLASS_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned char QNByte;
typedef unsigned int QNWord;

typedef struct QNClass QNClass;
typedef QNClass *QNClassRef;

typedef void * (* QNAllocatorFunc)(size_t);
typedef void (* QNFreeFunc)(const void *);
typedef const void *(* QNCopyFunc)(const void *);
typedef const void *(* QNRetainFunc)(const void *);
typedef void (* QNReleaseFunc)(const void *);
typedef void (* QNPrintFunc)(const void *);
typedef bool (* QNEqualFunc)(const void *, const void *);

#ifndef QNCLASSFIELDS

#define QNCLASSFIELDS const QNClassRef base; \
                      const char *name; \
                      size_t size; \
                      QNAllocatorFunc allocator; \
                      QNFreeFunc deallocate; \
                      QNCopyFunc copy; \
                      QNRetainFunc retain; \
                      QNReleaseFunc release; \
                      QNPrintFunc print; \
                      QNEqualFunc equal;

#endif

// indicate if memory is `strong` (allocated), should be free when not use anymore
#ifndef QN_STRONG
#define QN_STRONG
#endif

struct QNClass {
    QNCLASSFIELDS
};


extern const QNClassRef kQNBaseClassRef;

extern QN_STRONG void *QNAllocator(size_t size);

extern void QNDeallocate(const void *p);

#endif //QN_QNCLASS_H
