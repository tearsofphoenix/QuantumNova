//
// Created by Isaac on 2018/1/11.
//

#include "QCKeyPrivate.h"
#include "QCPrivate.h"

QCKeyRef QCKeyMalloc() {
    return _QCMalloc(sizeof(struct QCKey));
}

void QCKeyFree(QCKeyRef key) {
    _QCFree(key);
}

QCKeyRef QCKeyCreateWith(QCArrayRef h0, QCArrayRef h1, QCArrayRef h1inv, QCArrayRef g,
                         int length, int weight, int error) {
    QCKeyRef key = QCKeyMalloc();
    key->h0 = h0;
    key->h1 = h1;
    key->h1inv = h1inv;
    key->g = g;
    key->length = length;
    key->weight = weight;
    key->error = error;
    return key;
}