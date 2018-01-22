//
// Created by Isaac on 2018/1/12.
//

#ifndef QN_QNOBJECTPRIVATE_H
#define QN_QNOBJECTPRIVATE_H

#include "QNObject.h"

typedef struct _QNObject *QNObject;

extern QN_STRONG QNObjectRef _QNRetain(QNObjectRef object);

extern void _QNRelease(QNObjectRef object);

extern void _QNTagMemory(const void *p, const char *tag);

extern void _QNStartMemoryRecord();

extern void _QNPrintMemoryLeak();

#endif //QN_QNOBJECTPRIVATE_H
