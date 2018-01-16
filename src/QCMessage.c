//
// Created by Isaac on 2018/1/16.
//

#include "QCMessagePrivate.h"

static QCMessageRef QCMessageCopy(QCMessageRef message);

static struct QCClass kQCMessageClass = {
        .base = NULL,
        .name = "QCMessage",
        .allocator = QCAllocator,
        .deallocate = QCDeallocate,
        .size = sizeof(struct QCMessage),
        .copy = QCMessageCopy
};

// TODO
QCMessageRef QCMessageCreateFromPEMFile(const char *path) {

}

QCMessageRef QCMessageCreate(QCArrayRef c0, QCArrayRef c1, QCArrayRef sym) {
    QCMessageRef ref = QCAllocate(&kQCMessageClass);
    ref->c0 = QCRetain(c0);
    ref->c1 = QCRetain(c1);
    ref->sym = QCRetain(sym);
    return ref;
}

static QCMessageRef QCMessageCopy(QCMessageRef message) {
    QCMessageRef ref = QCAllocate(&kQCMessageClass);
    ref->c0 = QCObjectCopy(message->c0);
    ref->c1 = QCObjectCopy(message->c1);
    ref->sym = QCObjectCopy(message->sym);
    return ref;
}