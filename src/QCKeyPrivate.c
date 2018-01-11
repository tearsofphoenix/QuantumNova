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
