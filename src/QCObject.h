//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCOBJECT_H
#define PQC_CRYPTO_QCOBJECT_H

#include "QCClass.h"

#ifndef QCOBJECTFIELDS

#define QCOBJECTFIELDS QCClassRef isa; \
                      size_t retainCount; \

#endif

struct _QCObject {
    QCOBJECTFIELDS
};

typedef const void * QCObjectRef;

extern QCObjectRef QCAllocate(QCClassRef classRef);

extern QCObjectRef QCRetain(QCObjectRef object);

extern void QCRelease(QCObjectRef object);

extern QCObjectRef QCObjectCopy(QCObjectRef object);

extern bool QCObjectEqual(QCObjectRef object, QCObjectRef other);

extern void QCObjectPrint(QCObjectRef object);

#endif //PQC_CRYPTO_QCOBJECT_H
