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


// for memory leak debug

struct QCMemoryRecord {
    const void *p;
    const char *tag;
    size_t size;
};

typedef struct QCMemoryRecord QCMemoryRecord;

static const size_t kMaxCount = 48000;

static QCMemoryRecord kMemoryRecords[kMaxCount] = {{NULL, 0}};

static bool kNeedRecordMemory = false;

void _QCStartMemoryRecord() {
    kNeedRecordMemory = true;
}

void _QCPrintMemoryLeak() {
    kNeedRecordMemory = false;
    int total = 0;
    for (int i = 0; i < kMaxCount; ++i) {
        QCMemoryRecord *r = &kMemoryRecords[i];
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

static int _findFirstEmptyCell(QCMemoryRecord *list, size_t count) {
    for (int i = 0; i < count; ++i) {
        if (!list[i].p) {
            return i;
        }
    }
    return -1;
}

static int _findMemoryIndex(QCMemoryRecord *list, size_t count, const void *p) {
    for (int i = 0; i < count; ++i) {
        if (list[i].p == p) {
            return i;
        }
    }
    return -1;
}

void _QCTagMemory(const void *p, const char *tag) {
    if (kNeedRecordMemory) {
        int idx = _findMemoryIndex(kMemoryRecords, kMaxCount, p);
        if (idx != -1) {
            kMemoryRecords[idx].tag = tag;
        } else {
            printf("Warning: %p not in record!\n", p);
        }
    }
}

void *QCAllocator(size_t size) {
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

void QCDeallocate(const void *p) {
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
