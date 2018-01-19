//
// Created by Isaac on 2018/1/15.
//

#include <stdio.h>
#include <src/QCKeyPrivate.h>
#include "key-test.h"
#include "data.h"
#include "src/QNTest.h"
#include "src/QCArrayPrivate.h"
#include "src/QCMessagePrivate.h"
#include "src/QCCipherPrivate.h"

static bool private_key_test() {

    const char *path = "./aux/priv.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
    bool ret1 = QCArrayCompareRaw(key->h0, H0, QCDTDouble);
    bool ret2 = QCArrayCompareRaw(key->h1, H1, QCDTDouble);
    bool ret3 = QCArrayCompareRaw(key->h1inv, H1_inv, QCDTDouble);

    QCRelease(key);

    return ret1 && ret2 && ret3;
}

static bool public_key_test() {
    const char *path = "./aux/pub.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
    bool ret = QCArrayCompareRaw(key->g, G, QCDTDouble);
    QCRelease(key);

    return ret;
}

static bool message_test() {
    const char *path = "./aux/enc.data";
    QCMessageRef message = QCMessageCreateFromPEMFile(path);
    bool ret1 = QCArrayCompareRaw(message->c0, C0, QCDTDouble);
    bool ret2 = QCArrayCompareRaw(message->c1, C1, QCDTDouble);

    QCRelease(message);

    return ret1 && ret2;
}

static bool save_private_key() {
    const char *path = "./aux/priv.key";
    QCKeyRef key = QCKeyCreateFromPEMFile(path);
    const char *outpath = "./aux/priv-out.key";
    QCKeySaveToPEMFile(key, outpath);
}

static bool custom_create_key_test() {
    const char *path = "./aux/priv-c.key";
    const char *pubpath = "./aux/pub-c.key";
    QCKeyRef pubKey = NULL;
    QCKeyRef privKey = NULL;
    QCKeyGeneratePair(kQCDefaultKeyConfig, &privKey, &pubKey);

    bool ret1 = QCKeySaveToFile(privKey, path);
    bool ret2 = QCKeySaveToFile(pubKey, pubpath);

    QCRelease(privKey);
    QCRelease(pubKey);

    return ret1 && ret2;
}

static bool custom_load_key_test() {
    const char *path = "./aux/priv-c.key";
    const char *pubpath = "./aux/pub-c.key";
    QCKeyRef pubKey = QCKeyCreateFromFile(pubpath);
    QCKeyRef privKey = QCKeyCreateFromFile(path);


    QCByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QCCipherRef cipher = QCCipherCreate();
    QCCipherSetPrivateKey(cipher, privKey);
    QCCipherSetPublicKey(cipher, pubKey);

    QCArrayRef stream = QCArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QCMessageRef enc = QCCipherEncryptMessage(cipher, stream);

    QCArrayRef array = QCCipherDecryptMessage(cipher, enc);
    bool ret = QCArrayCompareRaw(array, msg, QCDTByte);

    QCRelease(privKey);
    QCRelease(pubKey);
    QCRelease(stream);
    QCRelease(enc);
    QCRelease(array);
    QCRelease(cipher);

    return ret;
}

void key_test() {

//    QNT("private key file", NULL, private_key_test, 1);
//
//    QNT("public key file", NULL, public_key_test, 1);
//
//    QNT("message file", NULL, message_test, 1);

    QNT("custom create", NULL, custom_create_key_test, 1);

    QNT("custom load", NULL, custom_load_key_test, 1);
}
