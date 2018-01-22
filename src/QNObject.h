//
// Created by Isaac on 2018/1/12.
//

#ifndef QN_QNOBJECT_H
#define QN_QNOBJECT_H

#include "QNClass.h"

#ifndef QNOBJECTFIELDS

#define QNOBJECTFIELDS QNClassRef isa; \
                      size_t retainCount; \

#endif

struct _QNObject {
    QNOBJECTFIELDS
};

typedef const void * QNObjectRef;

extern QN_STRONG QNObjectRef QNAllocate(QNClassRef classRef);

extern QN_STRONG QNObjectRef QNRetain(QNObjectRef object);

extern void QNRelease(QNObjectRef object);

extern QN_STRONG QNObjectRef QNObjectCopy(QNObjectRef object);

extern bool QNObjectEqual(QNObjectRef object, QNObjectRef other);

extern void QNObjectPrint(QNObjectRef object);

extern void QNLog(const char *fmt, ...);

#endif //QN_QNOBJECT_H
