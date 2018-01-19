//
// Created by Isaac on 2018/1/16.
//

#include "QCMessagePrivate.h"
#include "QCKeyPrivate.h"
#include "QCArrayPrivate.h"

#define kMessageLabel "PQP MESSAGE"

static QCMessageRef QCMessageCopy(QCMessageRef message);
static void QCMessageDeallocate(QCMessageRef message);
static void QCMessagePrint(QCMessageRef message);

static struct QCClass kQCMessageClass = {
        .base = NULL,
        .name = "QCMessage",
        .allocator = QCAllocator,
        .deallocate = QCMessageDeallocate,
        .size = sizeof(struct QCMessage),
        .copy = QCMessageCopy,
        .print = QCMessagePrint
};

static void QCMessageDeallocate(QCMessageRef message) {
    if (message) {
        QCRelease(message->c0);
        QCRelease(message->c1);
        QCRelease(message->sym);

        QCDeallocate(message);
    }
}

static QCMessageRef _parseMessageFile(const QCByte *data, size_t length) {

    ltc_asn1_list *decoded_list;
    size_t len;
    int ret = der_decode_sequence_flexi(data, &len, &decoded_list);
    QCArrayRef c0 = NULL, c1 = NULL, sym = NULL;
    if (ret != CRYPT_OK) {
        return NULL;
    }
    ltc_asn1_list *node = decoded_list->child;
    c0 = _decodeBitString(node);
    node = node->next;
    c1 = _decodeBitString(node);
    node = node->next;
    sym = _decodeOCTString(node);

    QCMessageRef message = QCMessageCreate(c0, c1, sym);

    QCRelease(c0);
    QCRelease(c1);
    QCRelease(sym);

    der_sequence_free(decoded_list);

    return message;
}

QCMessageRef QCMessageCreateFromPEMFile(const char *path) {
    size_t length = 0;
    QCByte *data = _readFileContent(path, &length);

    if (_isKindOfFile(data, kMessageLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kMessageLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        QCDeallocate(data);
        QCDeallocate(trimmed);

        QCMessageRef message =_parseMessageFile(array->data, array->count);
        QCRelease(array);
        return message;
    }
    return NULL;
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

static void QCMessagePrint(QCMessageRef message) {
    if (message) {
        printf("<%s %x> \n", message->isa->name, message);
        QCObjectPrint(message->c0);
        QCObjectPrint(message->c1);
        QCObjectPrint(message->sym);
    }
}