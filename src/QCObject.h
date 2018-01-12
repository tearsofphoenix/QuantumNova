//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCOBJECT_H
#define PQC_CRYPTO_QCOBJECT_H

#include "QCClass.h"

struct QCObject {
    QCClassRef isa;
    size_t retainCount;
};

typedef struct QCObject * QCObjectRef;

extern QCObjectRef QCRetain(QCObjectRef object);

extern void QCRelease(QCObjectRef object);

extern void QCObjectPrint(QCObjectRef object);

#endif //PQC_CRYPTO_QCOBJECT_H
