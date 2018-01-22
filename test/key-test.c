//
// Created by Isaac on 2018/1/15.
//

#include <stdio.h>
#include <src/QNKeyPrivate.h>
#include "key-test.h"
#include "data.h"
#include "src/QNTest.h"
#include "src/QNArrayPrivate.h"
#include "src/QNMessagePrivate.h"
#include "src/QNCipherPrivate.h"

static bool private_key_test() {

    const char *path = "./aux/priv.key";
    QNKeyRef key = QNKeyCreateFromPEMFile(path);
    bool ret1 = QNArrayCompareRaw(key->h0, H0, QNDTDouble);
    bool ret2 = QNArrayCompareRaw(key->h1, H1, QNDTDouble);
    bool ret3 = QNArrayCompareRaw(key->h1inv, H1_inv, QNDTDouble);

    QNRelease(key);

    return ret1 && ret2 && ret3;
}

static bool public_key_test() {
    const char *path = "./aux/pub.key";
    QNKeyRef key = QNKeyCreateFromPEMFile(path);
    bool ret = QNArrayCompareRaw(key->g, G, QNDTDouble);
    QNRelease(key);

    return ret;
}

static bool message_test() {
    const char *path = "./aux/enc.data";
    QNMessageRef message = QNMessageCreateFromPEMFile(path);
    bool ret1 = QNArrayCompareRaw(message->c0, C0, QNDTDouble);
    bool ret2 = QNArrayCompareRaw(message->c1, C1, QNDTDouble);

    QNRelease(message);

    return ret1 && ret2;
}

static bool save_private_key() {
    const char *path = "./aux/priv.key";
    QNKeyRef key = QNKeyCreateFromPEMFile(path);
    const char *outpath = "./aux/priv-out.key";
    QNKeySaveToPEMFile(key, outpath);
}

static bool custom_create_key_test() {
    const char *path = "./aux/priv-c.key";
    const char *pubpath = "./aux/pub-c.key";
    QNKeyRef pubKey = NULL;
    QNKeyRef privKey = NULL;
    QNKeyGeneratePair(kQNDefaultKeyConfig, &privKey, &pubKey);

    bool ret1 = QNKeySaveToFile(privKey, path);
    bool ret2 = QNKeySaveToFile(pubKey, pubpath);

    QNRelease(privKey);
    QNRelease(pubKey);

    return ret1 && ret2;
}

static bool custom_load_key_test() {
    const char *path = "./aux/priv-c.key";
    const char *pubpath = "./aux/pub-c.key";
    QNKeyRef pubKey = QNKeyCreateFromFile(pubpath);
    QNKeyRef privKey = QNKeyCreateFromFile(path);


    QNByte msg[] = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0a};

    QNCipherRef cipher = QNCipherCreate();
    QNCipherSetPrivateKey(cipher, privKey);
    QNCipherSetPublicKey(cipher, pubKey);

    QNArrayRef stream = QNArrayCreateWithByte(msg, sizeof(msg) / sizeof(msg[0]), true);
    QNMessageRef enc = QNCipherEncryptMessage(cipher, stream);

    QNArrayRef array = QNCipherDecryptMessage(cipher, enc);
    bool ret = QNArrayCompareRaw(array, msg, QNDTByte);

    QNRelease(privKey);
    QNRelease(pubKey);
    QNRelease(stream);
    QNRelease(enc);
    QNRelease(array);
    QNRelease(cipher);

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
