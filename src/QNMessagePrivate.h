//
// Created by Isaac on 2018/1/16.
//

#ifndef QN_QNMESSAGEPRIVATE_H
#define QN_QNMESSAGEPRIVATE_H

#include "QNObject.h"
#include "QNArray.h"
#include "QNMessage.h"

struct QNMessage {
    QNOBJECTFIELDS
    QNArrayRef c0;
    QNArrayRef c1;
    QNArrayRef sym;
};

extern QN_STRONG QNMessageRef QNMessageCreate(QNArrayRef c0, QNArrayRef c1, QNArrayRef sym);

#endif //QN_QNMESSAGEPRIVATE_H
