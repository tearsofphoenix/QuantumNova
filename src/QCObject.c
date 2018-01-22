//
// Created by Isaac on 2018/1/12.
//

#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "QCObject.h"
#include "QCObjectPrivate.h"

QCObjectRef QCAllocate(QCClassRef classRef) {
    if (classRef) {
        QCObject obj = classRef->allocator(classRef->size);
        obj->isa = classRef;
        obj->retainCount = 1;
        _QCTagMemory(obj, classRef->name);
        return obj;
    }
    return NULL;
}

QCObjectRef _QCRetain(QCObjectRef obj) {
    QCObject object = (QCObject)obj;
    if (object) {
        object->retainCount += 1;
    }
    return object;
}

void _QCRelease(QCObjectRef obj) {
    QCObject object = (QCObject)obj;
    if (object) {
        object->retainCount -= 1;
        if (object->retainCount == 0) {
            object->isa->deallocate(object);
        }
    }
}

QCObjectRef QCRetain(QCObjectRef obj) {
    QCObject object = (QCObject)obj;
    if (object) {
        if (object->isa->retain) {
            return object->isa->retain(object);
        } else {
            return _QCRetain(object);
        }
    }
    return object;
}

void QCRelease(QCObjectRef obj) {
    QCObject object = (QCObject)obj;
    if (object) {
        if (object->isa->release) {
            object->isa->release(object);
        } else {
            _QCRelease(object);
        }
    }
}

QCObjectRef QCObjectCopy(QCObjectRef obj) {
    QCObject object = (QCObject)obj;
    if (object) {
        return object->isa->copy(object);
    }
    return NULL;
}

bool QCObjectEqual(QCObjectRef obj, QCObjectRef other) {
    QCObject object = (QCObject)obj;
    if (object) {
        return object->isa->equal(object, other);
    }
    return false;
}


void QCObjectPrint(QCObjectRef obj) {
    QCObject object = (QCObject)obj;
    if (object) {
        object->isa->print(object);
    }
}

void QNLog(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t len = strlen(fmt);
    for (int i = 0; i < len; ++i) {
        char c = fmt[i];
        switch (c) {
            case '%': {
                ++i;
                char next = fmt[i];
                switch (next) {
                    case '%': {
                        printf("%%");
                        break;
                    }
                    case 'd': {
                        int d = va_arg(ap, int);
                        printf("%d", d);
                        break;
                    }
                    case 'f': {
                        double d = va_arg(ap, double);
                        printf("%f", d);
                        break;
                    }
                    case '@': {
                        QCObjectRef obj = va_arg(ap, QCObjectRef);
                        QCObjectPrint(obj);
                        break;
                    }
                    default: {
                        // ignore
                        break;
                    }
                }
                break;
            }
            default: {
                printf("%c", c);
                break;
            }
        }
    }
    va_end(ap);
}
