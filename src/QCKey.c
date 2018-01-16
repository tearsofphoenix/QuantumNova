//
// Created by Isaac on 2018/1/11.
//

#include "QCKey.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include "QCArrayPrivate.h"
#include <tomcrypt.h>
#include <math.h>
#include <memory.h>

QCKeyConfig kQCDefaultKeyConfig = {
        .length = 4801,
        .weight = 45,
        .error = 42
};

static void QCKeyDeallocate(QCKeyRef key);
static QCKeyRef QCKeyCopy(QCKeyRef key);
static void QCKeyPrint(QCKeyRef key);
static bool QCKeyEqual(QCKeyRef key1, QCKeyRef key2);

static struct QCClass kQCKeyClass = {
        .name = "QCKey",
        .allocator = QCAllocator,
        .size = sizeof(struct QCKey),
        .deallocate = QCKeyDeallocate,
        .copy = QCKeyCopy,
        .print = QCKeyPrint,
        .equal = QCKeyEqual
};

QCKeyRef QCKeyCreatePrivate(QCArrayRef h0, QCArrayRef h1, QCArrayRef h1inv, QCKeyConfig config) {
    QCKeyRef key = QCAllocate(&kQCKeyClass);
    key->h0 = QCRetain(h0);
    key->h1 = QCRetain(h1);
    key->h1inv = QCRetain(h1inv);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

QCKeyRef QCKeyCreatePublic(QCArrayRef g, QCKeyConfig config) {
    QCKeyRef key = QCAllocate(&kQCKeyClass);
    key->g = QCRetain(g);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

static QCKeyRef QCKeyCopy(QCKeyRef key) {
    if (key) {
        QCKeyRef copy = key->isa->allocator(sizeof(*key));
        copy->length = key->length;
        copy->weight = key->weight;
        copy->error = key->error;
        copy->h0 = (QCArrayRef)QCObjectCopy(key->h0);
        copy->h1 = (QCArrayRef)QCObjectCopy(key->h1);
        copy->h1inv = (QCArrayRef)QCObjectCopy(key->h1inv);
        copy->g = (QCArrayRef)QCObjectCopy(key->g);
        return copy;
    }
    return NULL;
}

static void QCKeyDeallocate(QCKeyRef key) {
    if (key) {
        QCRelease(key->h0);
        QCRelease(key->h1);
        QCRelease(key->h1inv);
        QCRelease(key->g);
    }
}

static void QCKeyPrint(QCKeyRef key) {
    if (key) {
        if (key->g) {
            printf("<QCPublicKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        } else {
            printf("<QCPrivateKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        }
    }
}

static bool QCKeyEqual(QCKeyRef key1, QCKeyRef key2) {
    if (key1 && key2 && key1->isa == key2->isa) {
        return key1->length == key2->length
               && key1->weight == key2->weight
               && key1->error == key2->error
               && QCObjectEqual(key1->h0, key2->h0)
               && QCObjectEqual(key1->h1, key2->h1)
               && QCObjectEqual(key1->h1inv, key2->h1inv)
               && QCObjectEqual(key1->g, key2->g);
    }
    return false;
}

void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey) {
    QCArrayRef h0 = QCRandomWeightVector(config.length, config.weight);
    QCArrayRef h1 = QCRandomWeightVector(config.length, config.weight);

    BN_CTX *bnCTX = BN_CTX_new();
    BIGNUM *base = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *n = BN_new();

    BN_dec2bn(&base, "2");
    BN_dec2bn(&exp, "1200");
    BN_exp(n, base, exp, bnCTX);
    BN_sub(n, n, base);

    printf("%s", BN_bn2dec(n));

    QCArrayRef h1inv = QCArrayExpPoly(h1, n);

    BN_free(base);
    BN_free(exp);
    BN_free(n);
    BN_CTX_free(bnCTX);

    QCKeyRef privKey = QCKeyCreatePrivate(h0, h1, h1inv, config);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);

    *privateKey = privKey;

    QCArrayRef g = QCArrayMulPoly(h0, h1inv);
    QCKeyRef pubKey = QCKeyCreatePublic(g, config);

    QCRelease(g);

    *publicKey = pubKey;
}

static QCKeyRef _parsePrivateKeyFile(const QCByte *data, size_t length) {
    size_t len2;
    ltc_asn1_list decoded_list[3];
    QCByte buf[kQCDefaultKeyConfig.length];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, buf, kQCDefaultKeyConfig.length);
    int err = der_decode_sequence(data, length, decoded_list, 1);
    if (err != CRYPT_OK) {
        return NULL;
    }

    size_t bufLength;
    if (decoded_list->type == LTC_ASN1_SEQUENCE) {
        ltc_asn1_list *child = decoded_list->child;
        QCArrayRef h0 = NULL;
        QCArrayRef h1 = NULL;
        QCArrayRef h1inv = NULL;

        if (child && child->type == LTC_ASN1_BIT_STRING) {
            der_decode_bit_string(child->data, child->size, buf, &bufLength);
            h0 = QCArrayCreateWithByte(buf, bufLength, true);
        }
        child = child->next;
        if (child != NULL && child->type == LTC_ASN1_BIT_STRING) {
            der_decode_bit_string(child->data, child->size, buf, &bufLength);
            h1 = QCArrayCreateWithByte(buf, bufLength, true);
        }
        child = child->next;
        if (child != NULL && child->type == LTC_ASN1_BIT_STRING) {
            der_decode_bit_string(child->data, child->size, buf, &bufLength);
            h1inv = QCArrayCreateWithByte(buf, bufLength, true);
        }
        QCKeyRef key = QCKeyCreatePrivate(h0, h1, h1inv, kQCDefaultKeyConfig);
        return key;
    }
    der_sequence_free(decoded_list);
    return NULL;
}

#define kBeginTemplate "-----BEGIN %s-----"
#define kEndTemplate "-----END %s-----"

static QCByte *_readFile(const char *path, size_t *outLength, const char *label) {
    char begin[1024] = {'\0'};
    sprintf(begin, kBeginTemplate, label);
    char end[1024] = {'\0'};
    sprintf(end, kEndTemplate, label);

    FILE *fileptr = fopen(path, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    size_t filelen = (size_t)ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    QCByte *buffer = malloc((filelen + 1) * sizeof(QCByte)); // Enough memory for file + \0

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    ssize_t total = 0;
    while ((read = getline(&line, &len, fileptr)) != -1) {
        // ignore '\n'
        memcpy(buffer + total, line, read - 1);
        total += read - 1;
    }
    if (line) {
        free(line);
    }

    fclose(fileptr); // Close the file

    const char *sch = strstr(buffer, begin);
    const char *ech = strstr(buffer, end);

    size_t bufferSize = (total - strlen(begin) - strlen(end));
    QCByte *result = malloc(sizeof(QCByte) * bufferSize);
    memcpy(result, buffer + strlen(begin), bufferSize);

    free(buffer);

    if (outLength) {
        *outLength = bufferSize;
    }

    return result;
}

QCKeyRef QCKeyCreateFromPEMFile(const char* filePath) {
    size_t length = 0;
    QCByte *data = _readFile(filePath, &length, "PQP PRIVATE KEY");

    QCArrayRef array = QCArrayCreateWithBase64(data, length);

    free(data);

    return _parsePrivateKeyFile(array->data, array->count);
}
