//
// Created by Isaac on 2018/1/11.
//

#include "QCKey.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include "QCArrayPrivate.h"
#include <tomcrypt.h>
#include <math.h>

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
    ltc_asn1_list decoded_list[3];
    size_t bs = 4801;
    QCByte h0buf[bs];
    QCByte h1buf[bs];
    QCByte h1invbuf[bs];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, h0buf, bs);
    LTC_SET_ASN1(decoded_list, 1, LTC_ASN1_BIT_STRING, h1buf, bs);
    LTC_SET_ASN1(decoded_list, 2, LTC_ASN1_BIT_STRING, h1invbuf, bs);
    der_decode_sequence(data, length, decoded_list, 3);

    QCByte buf[bs];
    size_t bufLength;

    ltc_asn1_list node = decoded_list[0];
    QCArrayRef h0 = NULL, h1 = NULL, h1inv = NULL;
    int ret;
    if (node.type == LTC_ASN1_BIT_STRING) {
        ret = der_decode_bit_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            h0 = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }
    node = decoded_list[1];
    if (node.type == LTC_ASN1_BIT_STRING) {
        ret = der_decode_bit_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            h1 = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }
    node = decoded_list[2];
    if (node.type == LTC_ASN1_BIT_STRING) {
        ret = der_decode_bit_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            h1inv = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }
    QCKeyRef privateKey = QCKeyCreatePrivate(h0, h1, h1inv, kQCDefaultKeyConfig);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);
    return privateKey;
}

static QCKeyRef _parsePublicKeyFile(const QCByte *data, size_t length) {
    ltc_asn1_list decoded_list[1];
    QCByte gbuf[4801];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, gbuf, 4801);
    der_decode_sequence(data, length, decoded_list, 1);

    QCByte buf[4801];
    size_t bufLength;

    ltc_asn1_list node = decoded_list[0];
    QCArrayRef g;
    if (node.type == LTC_ASN1_BIT_STRING) {
        int ret = der_decode_bit_string(node.data, node.size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            g = QCArrayCreateWithByte(buf, bufLength, true);
        } else {

        }
    }

    QCKeyRef key = QCKeyCreatePublic(g, kQCDefaultKeyConfig);
    QCRelease(g);

    return key;
}

#define kBeginTemplate "-----BEGIN %s-----"
#define kEndTemplate "-----END %s-----"
#define kPrivateKeyLabel "PQP PRIVATE KEY"
#define kPublicKeyLabel "PQP PUBLIC KEY"

bool _isKindOfFile(const char *fileContent, const char *label) {
    return strstr(fileContent, label) != NULL;
}

QCByte *_trimFileContent(const char *fileContent, size_t fileLength, size_t *outLength, const char *label) {
    char begin[64] = {'\0'};
    sprintf(begin, kBeginTemplate, label);
    char end[64] = {'\0'};
    sprintf(end, kEndTemplate, label);

    size_t bufferSize = (fileLength - strlen(begin) - strlen(end));
    size_t resultSize = sizeof(QCByte) * bufferSize;
    QCByte *result = malloc(resultSize + 1);
    result[resultSize] = '\0';
    memcpy(result, fileContent + strlen(begin), resultSize);

    if (outLength) {
        *outLength = bufferSize;
    }
    return result;
}

QCByte *_readFileContent(const char *path, size_t *outLength) {

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

    if (outLength) {
        *outLength = total;
    }

    fclose(fileptr); // Close the file

    return buffer;
}

QCKeyRef QCKeyCreateFromPEMFile(const char* filePath) {
    size_t length = 0;
    QCByte *data = _readFileContent(filePath, &length);

    if (_isKindOfFile(data, kPrivateKeyLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kPrivateKeyLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        free(data);
        free(trimmed);

        QCKeyRef key =_parsePrivateKeyFile(array->data, array->count);
        QCRelease(array);
        return key;
    } else if (_isKindOfFile(data, kPublicKeyLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kPublicKeyLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        free(data);
        free(trimmed);

        QCKeyRef key =_parsePublicKeyFile(array->data, array->count);
        QCRelease(array);
        return key;
    }
    return NULL;
}
