//
// Created by Isaac on 2018/1/12.
//

#include "QNClass.h"
#include "QNObjectPrivate.h"
#include <fftw3.h>
#include <memory.h>

static struct QNClass kBaseClass = {
        .name = "QNObject",
        .allocator = QNAllocator,
        .retain = QNRetain,
        .release = QNRelease,
        .copy = NULL,
        .deallocate = QNDeallocate,
};

const QNClassRef kQNBaseClassRef = &kBaseClass;


// for memory leak debug

struct QNMemoryRecord {
    const void *p;
    const char *tag;
    size_t size;
};

typedef struct QNMemoryRecord QNMemoryRecord;

#define kMaxCount 48000

static QNMemoryRecord kMemoryRecords[kMaxCount] = {{NULL, 0}};

static bool kNeedRecordMemory = false;

void _QNStartMemoryRecord() {
    kNeedRecordMemory = true;
}

void _QNPrintMemoryLeak() {
    kNeedRecordMemory = false;
    int total = 0;
    for (int i = 0; i < kMaxCount; ++i) {
        QNMemoryRecord *r = &kMemoryRecords[i];
        if (r->p) {
            total += r->size;
            if (r->tag) {
                printf("leak: <%s %p>, size: %d \n", r->tag, r->p, r->size);
            } else {
                printf("leak: %p, size: %d \n", r->p, r->size);
            }
        }
    }
    printf("total leak: %d bytes.\n", total);
}

static int _findFirstEmptyCell(QNMemoryRecord *list, size_t count) {
    for (int i = 0; i < count; ++i) {
        if (!list[i].p) {
            return i;
        }
    }
    return -1;
}

static int _findMemoryIndex(QNMemoryRecord *list, size_t count, const void *p) {
    for (int i = 0; i < count; ++i) {
        if (list[i].p == p) {
            return i;
        }
    }
    return -1;
}

void _QNTagMemory(const void *p, const char *tag) {
    if (kNeedRecordMemory) {
        int idx = _findMemoryIndex(kMemoryRecords, kMaxCount, p);
        if (idx != -1) {
            kMemoryRecords[idx].tag = tag;
        } else {
            printf("Warning: %p not in record!\n", p);
        }
    }
}

void *QNAllocator(size_t size) {
    void *mem = fftw_malloc(size);
    memset(mem, 0, size);
    if (kNeedRecordMemory) {
        int idx = _findFirstEmptyCell(kMemoryRecords, kMaxCount);
        if (idx != -1) {
            kMemoryRecords[idx].p = mem;
            kMemoryRecords[idx].size = size;
        }
    }
    return mem;
}

void QNDeallocate(const void *p) {
    if (kNeedRecordMemory) {
        int idx = _findMemoryIndex(kMemoryRecords, kMaxCount, p);
        if (idx != -1) {
            kMemoryRecords[idx].p = NULL;
            kMemoryRecords[idx].size = 0;
        } else {
            printf("Warning: %p not in record!\n", p);
        }
    }
    fftw_free(p);
}
