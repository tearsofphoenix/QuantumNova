//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCOBJECTPRIVATE_H
#define PQC_CRYPTO_QCOBJECTPRIVATE_H

#include "QCObject.h"

typedef struct _QCObject *QCObject;

extern QC_STRONG QCObjectRef _QCRetain(QCObjectRef object);

extern void _QCRelease(QCObjectRef object);

extern void _QCStartMemoryRecord();

extern void _QCPrintMemoryLeak();

#endif //PQC_CRYPTO_QCOBJECTPRIVATE_H
