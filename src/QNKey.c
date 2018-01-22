//
// Created by Isaac on 2018/1/11.
//

#include "QNKey.h"
#include "QNKeyPrivate.h"
#include "QNRandom.h"
#include "QNArrayPrivate.h"
#include <tomcrypt.h>
#include <math.h>

static const char *kPrivateKeyTag = "QNPVK";
static const char *kPublicKeyTag = "QNPBK";

/*
 * 80bit security
 */
QNKeyConfig kQNDefaultKeyConfig = {
        .length = 4801,
        .weight = 45,
        .error = 42
};

QNKeyConfig kQN128BitKeyConfig = {
        .length = 9857,
        .weight = 70,
        .error = 67,
};

/*
 * 256bit security key config
 */
QNKeyConfig kQN256BitKeyConfig = {
        .length = 32771,
        .weight = 135,
        .error = 132,
};

static void QNKeyDeallocate(QNObjectRef obj);
static QNKeyRef QNKeyCopy(QNKeyRef key);
static void QNKeyPrint(QNKeyRef key);
static bool QNKeyEqual(QNKeyRef key1, QNKeyRef key2);

static struct QNClass kQNKeyClass = {
        .name = "QNKey",
        .allocator = QNAllocator,
        .size = sizeof(struct QNKey),
        .deallocate = QNKeyDeallocate,
        .copy = QNKeyCopy,
        .print = QNKeyPrint,
        .equal = QNKeyEqual
};

QNKeyRef QNKeyCreatePrivate(QNArrayRef h0, QNArrayRef h1, QNArrayRef h1inv, QNKeyConfig config) {
    QNKeyRef key = QNAllocate(&kQNKeyClass);
    key->h0 = QNRetain(h0);
    key->h1 = QNRetain(h1);
    key->h1inv = QNRetain(h1inv);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

QNKeyRef QNKeyCreatePublic(QNArrayRef g, QNKeyConfig config) {
    QNKeyRef key = QNAllocate(&kQNKeyClass);
    key->g = QNRetain(g);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

static QNKeyRef QNKeyCopy(QNKeyRef key) {
    if (key) {
        QNKeyRef copy = key->isa->allocator(sizeof(*key));
        copy->length = key->length;
        copy->weight = key->weight;
        copy->error = key->error;
        copy->h0 = (QNArrayRef)QNObjectCopy(key->h0);
        copy->h1 = (QNArrayRef)QNObjectCopy(key->h1);
        copy->h1inv = (QNArrayRef)QNObjectCopy(key->h1inv);
        copy->g = (QNArrayRef)QNObjectCopy(key->g);
        return copy;
    }
    return NULL;
}

static void QNKeyDeallocate(QNObjectRef obj) {
    QNKeyRef key = obj;
    if (key) {
        QNRelease(key->h0);
        QNRelease(key->h1);
        QNRelease(key->h1inv);
        QNRelease(key->g);
        QNDeallocate(key);
    }
}

static void QNKeyPrint(QNKeyRef key) {
    if (key) {
        if (key->g) {
            printf("<QNPublicKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        } else {
            printf("<QNPrivateKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        }
    }
}

static bool QNKeyEqual(QNKeyRef key1, QNKeyRef key2) {
    if (key1 && key2 && key1->isa == key2->isa) {
        return key1->length == key2->length
               && key1->weight == key2->weight
               && key1->error == key2->error
               && QNObjectEqual(key1->h0, key2->h0)
               && QNObjectEqual(key1->h1, key2->h1)
               && QNObjectEqual(key1->h1inv, key2->h1inv)
               && QNObjectEqual(key1->g, key2->g);
    }
    return false;
}

void QNKeyGeneratePair(QNKeyConfig config, QNKeyRef *privateKey, QNKeyRef *publicKey) {
    QNArrayRef h0 = QNRandomWeightVector(config.length, config.weight);
    QNArrayRef h1 = QNRandomWeightVector(config.length, config.weight);

    BN_CTX *bnCTX = BN_CTX_new();
    BIGNUM *base = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *n = BN_new();

    BN_dec2bn(&base, "2");
    BN_dec2bn(&exp, "1200");
    BN_exp(n, base, exp, bnCTX);
    BN_sub(n, n, base);

    QNArrayRef h1inv = QNArrayExpPoly(h1, n);

    BN_free(base);
    BN_free(exp);
    BN_free(n);
    BN_CTX_free(bnCTX);

    QNKeyRef privKey = QNKeyCreatePrivate(h0, h1, h1inv, config);

    QNRelease(h0);
    QNRelease(h1);
    QNRelease(h1inv);

    *privateKey = privKey;

    QNArrayRef g = QNArrayMulPoly(h0, h1inv);
    QNKeyRef pubKey = QNKeyCreatePublic(g, config);

    QNRelease(g);

    *publicKey = pubKey;
}

QNArrayRef _decodeBitString(ltc_asn1_list *node) {
    QNByte buf[kQNDefaultKeyConfig.length];
    size_t size = node->size;
    memcpy(buf, node->data, size);
    return QNArrayCreateWithByte(buf, size, true);
}

QNArrayRef _decodeOCTString(ltc_asn1_list *node) {
    QNByte buf[kQNDefaultKeyConfig.length];
    size_t size = node->size;
    memcpy(buf, node->data, size);
    return QNArrayCreateWithByte(buf, size, true);
}

static QNKeyRef _parsePrivateKeyFile(const QNByte *data, size_t length) {

    ltc_asn1_list *decoded_list;
    size_t len;
    int ret = der_decode_sequence_flexi(data, &len, &decoded_list);
    QNArrayRef h0 = NULL;
    QNArrayRef h1 = NULL;
    QNArrayRef h1inv = NULL;

    if (ret != CRYPT_OK) {
        return NULL;
    }
    if (decoded_list->type == LTC_ASN1_SEQUENCE) {

        ltc_asn1_list *node = decoded_list->child;
        h0 = _decodeBitString(node);
        node = node->next;
        h1 = _decodeBitString(node);
        node = node->next;
        h1inv = _decodeBitString(node);

        QNKeyRef privateKey = QNKeyCreatePrivate(h0, h1, h1inv, kQNDefaultKeyConfig);

        der_sequence_free(decoded_list);

        QNRelease(h0);
        QNRelease(h1);
        QNRelease(h1inv);
        return privateKey;
    }
}

static QNKeyRef _parsePublicKeyFile(const QNByte *data, size_t length) {
    ltc_asn1_list *decoded_list;
    size_t len;
    int ret = der_decode_sequence_flexi(data, &len, &decoded_list);
    QNArrayRef g = NULL;

    if (ret != CRYPT_OK) {
        return NULL;
    }
    if (decoded_list->type == LTC_ASN1_SEQUENCE) {
        ltc_asn1_list *node = decoded_list->child;
        g = _decodeBitString(node);
    }

    QNKeyRef key = QNKeyCreatePublic(g, kQNDefaultKeyConfig);
    QNRelease(g);
    der_sequence_free(decoded_list);

    return key;
}

#define kBeginTemplate "-----BEGIN %s-----"
#define kEndTemplate "-----END %s-----"
#define kPrivateKeyLabel "PQP PRIVATE KEY"
#define kPublicKeyLabel "PQP PUBLIC KEY"

bool _isKindOfFile(const char *fileContent, const char *label) {
    return strstr(fileContent, label) != NULL;
}

QNByte *_trimFileContent(const char *fileContent, size_t fileLength, size_t *outLength, const char *label) {
    char begin[64] = {'\0'};
    sprintf(begin, kBeginTemplate, label);
    char end[64] = {'\0'};
    sprintf(end, kEndTemplate, label);

    size_t bufferSize = (fileLength - strlen(begin) - strlen(end));
    size_t resultSize = sizeof(QNByte) * bufferSize;
    QNByte *result = QNAllocator(resultSize + 1);
    result[resultSize] = '\0';
    memcpy(result, fileContent + strlen(begin), resultSize);

    if (outLength) {
        *outLength = bufferSize;
    }
    return result;
}

QNByte *_readFileContent(const char *path, size_t *outLength) {

    FILE *fileptr = fopen(path, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    size_t filelen = (size_t)ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    QNByte *buffer = QNAllocator((filelen + 1) * sizeof(QNByte)); // Enough memory for file + \0

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
        QNDeallocate(line);
    }

    if (outLength) {
        *outLength = total;
    }

    fclose(fileptr); // Close the file

    return buffer;
}

QNKeyRef QNKeyCreateFromPEMFile(const char* filePath) {
    size_t length = 0;
    QNByte *data = _readFileContent(filePath, &length);

    if (_isKindOfFile(data, kPrivateKeyLabel)) {
        QNByte *trimmed = _trimFileContent(data, length, &length, kPrivateKeyLabel);
        QNArrayRef array = QNArrayCreateWithBase64(trimmed, length);

        QNDeallocate(data);
        QNDeallocate(trimmed);

        QNKeyRef key =_parsePrivateKeyFile(array->data, array->count);
        QNRelease(array);
        return key;
    } else if (_isKindOfFile(data, kPublicKeyLabel)) {
        QNByte *trimmed = _trimFileContent(data, length, &length, kPublicKeyLabel);
        QNArrayRef array = QNArrayCreateWithBase64(trimmed, length);

        QNDeallocate(data);
        QNDeallocate(trimmed);

        QNKeyRef key =_parsePublicKeyFile(array->data, array->count);
        QNRelease(array);
        return key;
    }
    return NULL;
}


static void _savePrivateKeyToPath(QNKeyRef key, const char *path) {
    size_t count = key->h0->count + key->h1->count + key->h1inv->count;
    QNByte *buffer = _QNMallocData(QNDTByte, count, NULL);
    size_t idx = 0;
    size_t total = 0;
    der_encode_bit_string(key->h0->data, key->h0->count, buffer, &idx);
    total += idx;
    der_encode_bit_string(key->h1->data, key->h1->count, buffer + total, &idx);
    total += idx;
    der_encode_bit_string(key->h1inv->data, key->h1inv->count, buffer + total, &idx);
    total += idx;
    const char *b64string = QNEncodeBase64(buffer, total);

    FILE * fp;
    fp = fopen (path, "a");

    fprintf(fp, kBeginTemplate, kPrivateKeyLabel);
    fprintf(fp, "\n");
    fprintf(fp, b64string);
    fprintf(fp, kEndTemplate, kPrivateKeyLabel);
    fprintf(fp, "\n");

    /* close the file*/
    fclose (fp);

    QNDeallocate(buffer);
}


static void _savePublicKeyToPath(QNKeyRef key, const char *path) {
    size_t count = key->h0->count + key->h1->count + key->h1inv->count;
    QNByte *buffer = _QNMallocData(QNDTByte, count, NULL);
    size_t idx = 0;
    size_t total = 0;
    der_encode_bit_string(key->h0->data, key->h0->count, buffer, &idx);
    total += idx;
    der_encode_bit_string(key->h1->data, key->h1->count, buffer + total, &idx);
    total += idx;
    der_encode_bit_string(key->h1inv->data, key->h1inv->count, buffer + total, &idx);
    total += idx;
    const char *b64string = QNEncodeBase64(buffer, total);

    FILE * fp;
    fp = fopen (path, "a");

    fprintf(fp, kBeginTemplate, kPublicKeyLabel);
    fprintf(fp, "\n");
    fprintf(fp, b64string);
    fprintf(fp, kEndTemplate, kPublicKeyLabel);
    fprintf(fp, "\n");

    /* close the file*/
    fclose (fp);

    QNDeallocate(buffer);
}

void QNKeySaveToPEMFile(QNKeyRef key, const char *path) {
    if (key && path) {
        if (key->h0) {
            // is private key
            _savePrivateKeyToPath(key, path);
        } else if (key->g) {
            // is public key
            _savePublicKeyToPath(key, path);
        }
    }
}

/*
 * custom save & parse
 */
bool QNKeySaveToFile(QNKeyRef key, const char *path) {
    FILE *f = fopen(path, "wb");
    const char *tag = NULL;
    if (key->g) {
        // public key
        tag = kPublicKeyTag;
        fwrite(tag, sizeof(char), strlen(tag), f);

        fwrite(&key->length, sizeof(size_t), 1, f);
        fwrite(&key->weight, sizeof(size_t), 1, f);
        fwrite(&key->error, sizeof(size_t), 1, f);

        QNArraySaveToFile(key->g, f);
    } else {
        tag = kPrivateKeyTag;
        fwrite(tag, sizeof(char), strlen(tag), f);

        fwrite(&key->length, sizeof(size_t), 1, f);
        fwrite(&key->weight, sizeof(size_t), 1, f);
        fwrite(&key->error, sizeof(size_t), 1, f);

        QNArraySaveToFile(key->h0, f);
        QNArraySaveToFile(key->h1, f);
        QNArraySaveToFile(key->h1inv, f);
    }

    fclose(f);
    return true;
}

QNKeyRef QNKeyCreateFromFile(const char *path) {
    FILE *f = fopen(path, "rb");
    size_t len = 5;
    const char *tag = QNAllocator(sizeof(char) * (5 + 1));
    fread(tag, sizeof(char), len, f);
    if (strcmp(tag, kPrivateKeyTag) == 0) {

        size_t length = 0;
        fread(&length, sizeof(size_t), 1, f);
        size_t weight = 0;
        fread(&weight, sizeof(size_t), 1, f);
        size_t error = 0;
        fread(&error, sizeof(size_t), 1, f);

        // private key
        QNArrayRef h0 = QNArrayFromFile(f);
        QNArrayRef h1 = QNArrayFromFile(f);
        QNArrayRef h1inv = QNArrayFromFile(f);

        QNKeyConfig config = {length, weight, error};
        QNKeyRef key = QNKeyCreatePrivate(h0, h1, h1inv, config);
        QNRelease(h0);
        QNRelease(h1);
        QNRelease(h1inv);

        fclose(f);
        QNDeallocate(tag);

        return key;
    } else {

        size_t length = 0;
        fread(&length, sizeof(size_t), 1, f);
        size_t weight = 0;
        fread(&weight, sizeof(size_t), 1, f);
        size_t error = 0;
        fread(&error, sizeof(size_t), 1, f);

        QNArrayRef g = QNArrayFromFile(f);

        QNKeyConfig config = {length, weight, error};
        QNKeyRef pubkey = QNKeyCreatePublic(g, config);
        QNRelease(g);

        fclose(f);
        QNDeallocate(tag);

        return pubkey;
    }

}
