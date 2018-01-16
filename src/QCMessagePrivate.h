//
// Created by Isaac on 2018/1/16.
//

#ifndef PQC_CRYPTO_QCMESSAGEPRIVATE_H
#define PQC_CRYPTO_QCMESSAGEPRIVATE_H

#include "QCObject.h"
#include "QCArray.h"
#include "QCMessage.h"

struct QCMessage {
    QCOBJECTFIELDS
    QCArrayRef c0;
    QCArrayRef c1;
    QCArrayRef sym;
};

QCMessageRef QCMessageCreate(QCArrayRef c0, QCArrayRef c1, QCArrayRef sym);

#endif //PQC_CRYPTO_QCMESSAGEPRIVATE_H
