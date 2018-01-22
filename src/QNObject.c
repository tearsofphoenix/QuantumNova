//
// Created by Isaac on 2018/1/12.
//

#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "QNObject.h"
#include "QNObjectPrivate.h"

QNObjectRef QNAllocate(QNClassRef classRef) {
    if (classRef) {
        QNObject obj = classRef->allocator(classRef->size);
        obj->isa = classRef;
        obj->retainCount = 1;
        _QNTagMemory(obj, classRef->name);
        return obj;
    }
    return NULL;
}

QNObjectRef _QNRetain(QNObjectRef obj) {
    QNObject object = (QNObject)obj;
    if (object) {
        object->retainCount += 1;
    }
    return object;
}

void _QNRelease(QNObjectRef obj) {
    QNObject object = (QNObject)obj;
    if (object) {
        object->retainCount -= 1;
        if (object->retainCount == 0) {
            object->isa->deallocate(object);
        }
    }
}

QNObjectRef QNRetain(QNObjectRef obj) {
    QNObject object = (QNObject)obj;
    if (object) {
        if (object->isa->retain) {
            return object->isa->retain(object);
        } else {
            return _QNRetain(object);
        }
    }
    return object;
}

void QNRelease(QNObjectRef obj) {
    QNObject object = (QNObject)obj;
    if (object) {
        if (object->isa->release) {
            object->isa->release(object);
        } else {
            _QNRelease(object);
        }
    }
}

QNObjectRef QNObjectCopy(QNObjectRef obj) {
    QNObject object = (QNObject)obj;
    if (object) {
        return object->isa->copy(object);
    }
    return NULL;
}

bool QNObjectEqual(QNObjectRef obj, QNObjectRef other) {
    QNObject object = (QNObject)obj;
    if (object) {
        return object->isa->equal(object, other);
    }
    return false;
}


void QNObjectPrint(QNObjectRef obj) {
    QNObject object = (QNObject)obj;
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
                        QNObjectRef obj = va_arg(ap, QNObjectRef);
                        QNObjectPrint(obj);
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
