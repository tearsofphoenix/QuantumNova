//
// Created by Isaac on 2018/1/16.
//

#include "QNMessagePrivate.h"
#include "QNKeyPrivate.h"
#include "QNArrayPrivate.h"

#define kMessageLabel "PQP MESSAGE"

static QNMessageRef QNMessageCopy(QNMessageRef message);
static void QNMessageDeallocate(QNMessageRef message);
static void QNMessagePrint(QNMessageRef message);

static struct QNClass kQNMessageClass = {
        .base = NULL,
        .name = "QNMessage",
        .allocator = QNAllocator,
        .deallocate = QNMessageDeallocate,
        .size = sizeof(struct QNMessage),
        .copy = QNMessageCopy,
        .print = QNMessagePrint
};

static void QNMessageDeallocate(QNMessageRef message) {
    if (message) {
        QNRelease(message->c0);
        QNRelease(message->c1);
        QNRelease(message->sym);

        QNDeallocate(message);
    }
}

static QNMessageRef _parseMessageFile(const QNByte *data, size_t length) {

    ltc_asn1_list *decoded_list;
    size_t len;
    int ret = der_decode_sequence_flexi(data, &len, &decoded_list);
    QNArrayRef c0 = NULL, c1 = NULL, sym = NULL;
    if (ret != CRYPT_OK) {
        return NULL;
    }
    ltc_asn1_list *node = decoded_list->child;
    c0 = _decodeBitString(node);
    node = node->next;
    c1 = _decodeBitString(node);
    node = node->next;
    sym = _decodeOCTString(node);

    QNMessageRef message = QNMessageCreate(c0, c1, sym);

    QNRelease(c0);
    QNRelease(c1);
    QNRelease(sym);

    der_sequence_free(decoded_list);

    return message;
}

QNMessageRef QNMessageCreateFromPEMFile(const char *path) {
    size_t length = 0;
    QNByte *data = _readFileContent(path, &length);

    if (_isKindOfFile(data, kMessageLabel)) {
        QNByte *trimmed = _trimFileContent(data, length, &length, kMessageLabel);
        QNArrayRef array = QNArrayCreateWithBase64(trimmed, length);

        QNDeallocate(data);
        QNDeallocate(trimmed);

        QNMessageRef message =_parseMessageFile(array->data, array->count);
        QNRelease(array);
        return message;
    }
    return NULL;
}

QNMessageRef QNMessageCreate(QNArrayRef c0, QNArrayRef c1, QNArrayRef sym) {
    QNMessageRef ref = QNAllocate(&kQNMessageClass);
    ref->c0 = QNRetain(c0);
    ref->c1 = QNRetain(c1);
    ref->sym = QNRetain(sym);
    return ref;
}

static QNMessageRef QNMessageCopy(QNMessageRef message) {
    QNMessageRef ref = QNAllocate(&kQNMessageClass);
    ref->c0 = QNObjectCopy(message->c0);
    ref->c1 = QNObjectCopy(message->c1);
    ref->sym = QNObjectCopy(message->sym);
    return ref;
}

static void QNMessagePrint(QNMessageRef message) {
    if (message) {
        printf("<%s %x> \n", message->isa->name, message);
        QNObjectPrint(message->c0);
        QNObjectPrint(message->c1);
        QNObjectPrint(message->sym);
    }
}