//
// Created by Isaac on 2018/1/12.
//

#include "QCObject.h"

QCObjectRef _QCRetain(QCObjectRef object) {
    if (object) {
        object->retainCount += 1;
    }
    return object;
}

void _QCRelease(QCObjectRef object) {
    if (object) {
        object->retainCount -= 1;
        if (object->retainCount == 0) {
            object->isa->deallocate(object);
        }
    }
}

QCObjectRef QCRetain(QCObjectRef object) {
    if (object) {
        return object->isa->retain(object);
    }
    return object;
}

void QCRelease(QCObjectRef object) {
    if (object) {
        object->isa->release(object);
    }
}

void QCObjectPrint(QCObjectRef object) {
    if (object) {
        object->isa->print(object);
    }
}
