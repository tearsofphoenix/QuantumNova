//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCOBJECTPRIVATE_H
#define PQC_CRYPTO_QCOBJECTPRIVATE_H

#include "QCObject.h"

typedef struct _QCObject *QCObject;

extern QCObjectRef _QCRetain(QCObjectRef object);

extern void _QCRelease(QCObjectRef object);

#endif //PQC_CRYPTO_QCOBJECTPRIVATE_H
