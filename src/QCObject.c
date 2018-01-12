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
        if (object->isa->retain) {
            return object->isa->retain(object);
        } else {
            return _QCRetain(object);
        }
    }
    return object;
}

void QCRelease(QCObjectRef object) {
    if (object) {
        if (object->isa->release) {
            object->isa->release(object);
        } else {
            _QCRelease(object);
        }
    }
}

QCObjectRef QCObjectCopy(QCObjectRef object) {
    if (object) {
        return object->isa->copy(object);
    }
    return NULL;
}

bool QCObjectEqual(QCObjectRef object, QCObjectRef other) {
    if (object) {
        return object->isa->equal(object, object);
    }
    return false;
}


void QCObjectPrint(QCObjectRef object) {
    if (object) {
        object->isa->print(object);
    }
}
