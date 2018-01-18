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

    size_t bs = kQCDefaultKeyConfig.length;
    QCByte *h0buf = QCAllocator(sizeof(QCByte) * bs);
    QCByte *h1buf = QCAllocator(sizeof(QCByte) * bs);
    QCByte *h1invbuf = QCAllocator(sizeof(QCByte) * bs);

    const unsigned char _der_tests_cacert_root_cert[] =
            "MIIHPTCCBSWgAwIBAgIBADANBgkqhkiG9w0BAQQFADB5MRAwDgYDVQQKEwdSb290"
                    "IENBMR4wHAYDVQQLExVodHRwOi8vd3d3LmNhY2VydC5vcmcxIjAgBgNVBAMTGUNB"
                    "IENlcnQgU2lnbmluZyBBdXRob3JpdHkxITAfBgkqhkiG9w0BCQEWEnN1cHBvcnRA"
                    "Y2FjZXJ0Lm9yZzAeFw0wMzAzMzAxMjI5NDlaFw0zMzAzMjkxMjI5NDlaMHkxEDAO"
                    "BgNVBAoTB1Jvb3QgQ0ExHjAcBgNVBAsTFWh0dHA6Ly93d3cuY2FjZXJ0Lm9yZzEi"
                    "MCAGA1UEAxMZQ0EgQ2VydCBTaWduaW5nIEF1dGhvcml0eTEhMB8GCSqGSIb3DQEJ"
                    "ARYSc3VwcG9ydEBjYWNlcnQub3JnMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIIC"
                    "CgKCAgEAziLA4kZ97DYoB1CW8qAzQIxL8TtmPzHlawI229Z89vGIj053NgVBlfkJ"
                    "8BLPRoZzYLdufujAWGSuzbCtRRcMY/pnCujW0r8+55jE8Ez64AO7NV1sId6eINm6"
                    "zWYyN3L69wj1x81YyY7nDl7qPv4coRQKFWyGhFtkZip6qUtTefWIonvuLwphK42y"
                    "fk1WpRPs6tqSnqxEQR5YYGUFZvjARL3LlPdCfgv3ZWiYUQXw8wWRBB0bF4LsyFe7"
                    "w2t6iPGwcswlWyCR7BYCEo8y6RcYSNDHBS4CMEK4JZwFaz+qOqfrU0j36NK2B5jc"
                    "G8Y0f3/JHIJ6BVgrCFvzOKKrF11myZjXnhCLotLddJr3cQxyYN/Nb5gznZY0dj4k"
                    "epKwDpUeb+agRThHqtdB7Uq3EvbXG4OKDy7YCbZZ16oE/9KTfWgu3YtLq1i6L43q"
                    "laegw1SJpfvbi1EinbLDvhG+LJGGi5Z4rSDTii8aP8bQUWWHIbEZAWV/RRyH9XzQ"
                    "QUxPKZgh/TMfdQwEUfoZd9vUFBzugcMd9Zi3aQaRIt0AUMyBMawSB3s42mhb5ivU"
                    "fslfrejrckzzAeVLIL+aplfKkQABi6F1ITe1Yw1nPkZPcCBnzsXWWdsC4PDSy826"
                    "YreQQejdIOQpvGQpQsgi3Hia/0PsmBsJUUtaWsJx8cTLc6nloQsCAwEAAaOCAc4w"
                    "ggHKMB0GA1UdDgQWBBQWtTIb1Mfz4OaO873SsDrusjkY0TCBowYDVR0jBIGbMIGY"
                    "gBQWtTIb1Mfz4OaO873SsDrusjkY0aF9pHsweTEQMA4GA1UEChMHUm9vdCBDQTEe"
                    "MBwGA1UECxMVaHR0cDovL3d3dy5jYWNlcnQub3JnMSIwIAYDVQQDExlDQSBDZXJ0"
                    "IFNpZ25pbmcgQXV0aG9yaXR5MSEwHwYJKoZIhvcNAQkBFhJzdXBwb3J0QGNhY2Vy"
                    "dC5vcmeCAQAwDwYDVR0TAQH/BAUwAwEB/zAyBgNVHR8EKzApMCegJaAjhiFodHRw"
                    "czovL3d3dy5jYWNlcnQub3JnL3Jldm9rZS5jcmwwMAYJYIZIAYb4QgEEBCMWIWh0"
                    "dHBzOi8vd3d3LmNhY2VydC5vcmcvcmV2b2tlLmNybDA0BglghkgBhvhCAQgEJxYl"
                    "aHR0cDovL3d3dy5jYWNlcnQub3JnL2luZGV4LnBocD9pZD0xMDBWBglghkgBhvhC"
                    "AQ0ESRZHVG8gZ2V0IHlvdXIgb3duIGNlcnRpZmljYXRlIGZvciBGUkVFIGhlYWQg"
                    "b3ZlciB0byBodHRwOi8vd3d3LmNhY2VydC5vcmcwDQYJKoZIhvcNAQEEBQADggIB"
                    "ACjH7pyCArpcgBLKNQodgW+JapnM8mgPf6fhjViVPr3yBsOQWqy1YPaZQwGjiHCc"
                    "nWKdpIevZ1gNMDY75q1I08t0AoZxPuIrA2jxNGJARjtT6ij0rPtmlVOKTV39O9lg"
                    "18p5aTuxZZKmxoGCXJzN600BiqXfEVWqFcofN8CCmHBh22p8lqOOLlQ+TyGpkO/c"
                    "gr/c6EWtTZBzCDyUZbAEmXZ/4rzCahWqlwQ3JNgelE5tDlG+1sSPypZt90Pf6DBl"
                    "Jzt7u0NDY8RD97LsaMzhGY4i+5jhe1o+ATc7iwiwovOVThrLm82asduycPAtStvY"
                    "sONvRUgzEv/+PDIqVPfE94rwiCPCR/5kenHA0R6mY7AHfqQv0wGP3J8rtsYIqQ+T"
                    "SCX8Ev2fQtzzxD72V7DX3WnRBnc0CkvSyqD/HMaMyRa+xMwyN2hzXwj7UfdJUzYF"
                    "CpUCTPJ5GhD22Dp1nPMd8aINcGeGG7MW9S/lpOt5hvk9C8JzC6WZrG/8Z7jlLwum"
                    "GCSNe9FINSkYQKyTYOGWhlC0elnYjyELn8+CkcY7v2vcB5G5l1YjqrZslMZIBjzk"
                    "zk6q5PYvCdxTby78dOs6Y5nCpqyJvKeyRKANihDjbPIky/qbn3BHLt4Ui9SyIAmW"
                    "omTxJBzcoTWcFbLUvFUufQb1nA5V9FrWk9p2rSVzTMVD";
    const unsigned long _der_tests_cacert_root_cert_size = sizeof(_der_tests_cacert_root_cert);
    unsigned char buf[_der_tests_cacert_root_cert_size];
    size_t len1 = sizeof(buf);
    base64_decode(_der_tests_cacert_root_cert, sizeof(_der_tests_cacert_root_cert), buf, &len1);

    ltc_asn1_list *decoded_list;
    size_t len = 0;

    der_decode_sequence_flexi(buf, &len, &decoded_list);

    QCByte *buf2 = QCAllocator(sizeof(QCByte) * bs);
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
    size_t size = kQCDefaultKeyConfig.length;
    QCByte gbuf[size];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, gbuf, size);
    der_decode_sequence(data, length, decoded_list, 1);

    QCByte buf[size];
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
    QCByte *result = QCAllocator(resultSize + 1);
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

    QCByte *buffer = QCAllocator((filelen + 1) * sizeof(QCByte)); // Enough memory for file + \0

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
        QCDeallocate(line);
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

        QCDeallocate(data);
        QCDeallocate(trimmed);

        QCObjectPrint(array);

        QCKeyRef key =_parsePrivateKeyFile(array->data, array->count);
        QCRelease(array);
        return key;
    } else if (_isKindOfFile(data, kPublicKeyLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kPublicKeyLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        QCDeallocate(data);
        QCDeallocate(trimmed);

        QCKeyRef key =_parsePublicKeyFile(array->data, array->count);
        QCRelease(array);
        return key;
    }
    return NULL;
}


static void _savePrivateKeyToPath(QCKeyRef key, const char *path) {
    size_t count = key->h0->count + key->h1->count + key->h1inv->count;
    QCByte *buffer = _QCMallocData(QCDTByte, count, NULL);
    size_t idx = 0;
    size_t total = 0;
    der_encode_bit_string(key->h0->data, key->h0->count, buffer, &idx);
    total += idx;
    der_encode_bit_string(key->h1->data, key->h1->count, buffer + total, &idx);
    total += idx;
    der_encode_bit_string(key->h1inv->data, key->h1inv->count, buffer + total, &idx);
    total += idx;
    const char *b64string = QCEncodeBase64(buffer, total);

    FILE * fp;
    fp = fopen (path, "a");

    fprintf(fp, kBeginTemplate, kPrivateKeyLabel);
    fprintf(fp, "\n");
    fprintf(fp, b64string);
    fprintf(fp, kEndTemplate, kPrivateKeyLabel);
    fprintf(fp, "\n");

    /* close the file*/
    fclose (fp);

    QCDeallocate(buffer);
}


static void _savePublicKeyToPath(QCKeyRef key, const char *path) {
    size_t count = key->h0->count + key->h1->count + key->h1inv->count;
    QCByte *buffer = _QCMallocData(QCDTByte, count, NULL);
    size_t idx = 0;
    size_t total = 0;
    der_encode_bit_string(key->h0->data, key->h0->count, buffer, &idx);
    total += idx;
    der_encode_bit_string(key->h1->data, key->h1->count, buffer + total, &idx);
    total += idx;
    der_encode_bit_string(key->h1inv->data, key->h1inv->count, buffer + total, &idx);
    total += idx;
    const char *b64string = QCEncodeBase64(buffer, total);

    FILE * fp;
    fp = fopen (path, "a");

    fprintf(fp, kBeginTemplate, kPublicKeyLabel);
    fprintf(fp, "\n");
    fprintf(fp, b64string);
    fprintf(fp, kEndTemplate, kPublicKeyLabel);
    fprintf(fp, "\n");

    /* close the file*/
    fclose (fp);

    QCDeallocate(buffer);
}

void QCKeySaveToFile(QCKeyRef key, const char *path) {
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