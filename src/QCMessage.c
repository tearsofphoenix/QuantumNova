//
// Created by Isaac on 2018/1/16.
//

#include "QCMessagePrivate.h"
#include "QCKeyPrivate.h"
#include "QCArrayPrivate.h"
#include <tomcrypt.h>

#define kMessageLabel "PQP MESSAGE"

static QCMessageRef QCMessageCopy(QCMessageRef message);

static void QCMessagePrint(QCMessageRef message);

static struct QCClass kQCMessageClass = {
        .base = NULL,
        .name = "QCMessage",
        .allocator = QCAllocator,
        .deallocate = QCDeallocate,
        .size = sizeof(struct QCMessage),
        .copy = QCMessageCopy,
        .print = QCMessagePrint
};


static QCMessageRef _parseMessageFile(const QCByte *data, size_t length) {
    ltc_asn1_list decoded_list[3];
    size_t bs = 4801;
    QCByte c0buf[bs];
    QCByte c1buf[bs];
    QCByte symbuf[bs];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, c0buf, bs);
    LTC_SET_ASN1(decoded_list, 1, LTC_ASN1_BIT_STRING, c1buf, bs);
    LTC_SET_ASN1(decoded_list, 2, LTC_ASN1_BIT_STRING, symbuf, bs);
    der_decode_sequence(data, length, decoded_list, 3);

    QCByte buf[bs];
    size_t bufLength;

    ltc_asn1_list node = decoded_list[0];
    QCArrayRef c0 = NULL, c1 = NULL, sym = NULL;
    int ret;
    if (node.type == LTC_ASN1_BIT_STRING) {
        ret = der_decode_bit_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            c0 = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }
    node = decoded_list[1];
    if (node.type == LTC_ASN1_BIT_STRING) {
        ret = der_decode_bit_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            c1 = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }
    node = decoded_list[2];
    if (node.type == LTC_ASN1_OCTET_STRING) {
        ret = der_decode_octet_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            sym = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }

    QCMessageRef message = QCMessageCreate(c0, c1, sym);

    QCRelease(c0);
    QCRelease(c1);
    QCRelease(sym);

    return message;
}

QCMessageRef QCMessageCreateFromPEMFile(const char *path) {
    size_t length = 0;
    QCByte *data = _readFileContent(path, &length);

    if (_isKindOfFile(data, kMessageLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kMessageLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        free(data);
        free(trimmed);

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