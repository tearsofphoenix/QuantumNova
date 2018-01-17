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
    ltc_asn1_list decoded_list[3];
    QCByte h0buf[kQCDefaultKeyConfig.length];
    QCByte h1buf[kQCDefaultKeyConfig.length];
    QCByte h1invbuf[kQCDefaultKeyConfig.length];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, h0buf, kQCDefaultKeyConfig.length);
    LTC_SET_ASN1(decoded_list, 1, LTC_ASN1_BIT_STRING, h1buf, kQCDefaultKeyConfig.length);
    LTC_SET_ASN1(decoded_list, 2, LTC_ASN1_BIT_STRING, h1invbuf, kQCDefaultKeyConfig.length);
    der_decode_sequence(data, length, decoded_list, 3);

    QCByte buf[kQCDefaultKeyConfig.length];
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
    return privateKey;
}

static QCKeyRef _parsePublicKeyFile(const QCByte *data, size_t length) {
    ltc_asn1_list decoded_list[1];
    QCByte gbuf[kQCDefaultKeyConfig.length];
    LTC_SET_ASN1(decoded_list, 0, LTC_ASN1_BIT_STRING, gbuf, kQCDefaultKeyConfig.length);
    der_decode_sequence(data, length, decoded_list, 1);

    QCByte buf[kQCDefaultKeyConfig.length];
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

    QCKeyRef privateKey = QCKeyCreatePublic(g, kQCDefaultKeyConfig);
    der_sequence_free(decoded_list);
    return privateKey;
}

#define kBeginTemplate "-----BEGIN %s-----"
#define kEndTemplate "-----END %s-----"
#define kPrivateKeyLabel "PQP PRIVATE KEY"
#define kPublicKeyLabel "PQP PUBLIC KEY"
#define kMessageLabel "PQP MESSAGE"

static bool _isKindOfFile(const char *fileContent, const char *label) {
    return strstr(fileContent, label) != NULL;
}

static QCByte *_trimFileContent(const char *fileContent, size_t fileLength, size_t *outLength, const char *label) {
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

static QCByte *_readFileContent(const char *path, size_t *outLength) {

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
//        const char *str = "3082071A0382025A07000000020000400000000000000000000000000000000000000000000000802000000000001000000000000000000080000000000000000000000000000000000100000000000000000000200000000000000000000000000000040000200000000000000000000000000000000000000000000000000000080000000000100000000400000002000004000000000200000000000000000000000000000000000000000000400400000004000000000000000000000000000000000000000000000000000080000008001000000000000000000000080000000000000000080000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011000100000000000000400000000000000000000000000000000000000000000000040000000000000000000000000800000000000000000000000000000000000000000000000000000000000040000000000000002400000000000000000000002000000000200001000000000000000000000801000000000000000000000000000000000000000000000000000000000000000000000000000800010002000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000100000000000000000382025A07000001000000010000000000000010000000000000000000200000000000000000000000000000000000000000000000000000000000000000000000000000000000200000000010008000000004000040000000000006000000000000000000000000000400000000000000000020000002020002000000000000000000200000000000000000000000000000000000000000004000400000000000000000000000000000000000000000000000000000000000000000000000000000000000000020040000000000000000000000000000000000000000000000000000000000000000000000000000000020200000000000000000000000000200000000000000004000000000000000000080000000000000000000000000000000000000000000000000004000020000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000001000000000000000000000400000000000000010000000000000000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000400000000000000000000000000000008000000000000000080000000000001000000000000000000000000000000000000000000000000000000001000000000000800000000000000000000010400000100100000000000000000000000000000382025A0736BE8804C1BA7B635F53A9182943D2805008370E25FEEBE7CDA62BCE8216F7EA6BFE4DB6E9E49B6221D8E5F6FE54F0E9A5A2A804742426F2FAF867348143442AFDBF2C71DD45E342C0FC2854F8D70508905FE024A36C8DD3B577097D39755DD7DB87B9CB6167103DA95D45371C04ECC53F7C0D3D3FE3899F5C3182FE4A8DFE4797AC9C390B02D1C605D72D28C1B85530CF0FC4C9747D47EB427E8A9C564DD1D544685BB0B7366885690841C8E360415C1230364D5686A5C98B297CB30A9DE9971F0FA740BD4B2F4A6A324F7718339C7A0EC637EF0DEDF498D0F2F17EF1B5703DE47777B434B9B5D5CB132340C8F5481596DA34AA6D675738F6605EC4657515264DB9CC4CCF278B7CB335866BDED2B42BF494164753C3C1918D22E68CD6A37852EAAC880B3F6648A05419DF435E755D8840078E8912E0A7B92606F88C663A1FD2FC88B0D459CF364466D4B9DE2E039670BAB912FEB495C2AF6881EF5BA3FD36C53BB663A7931FDF971988CAD21563092451081C6847C4CF5C81B5643C7ED44DC46F516DC7AA373ABA87C5D61572348DA47E620CDC0600C821A50BBB63F0C53B305ED501B5CD7AD2E37B47463AEB1ED056AED1CBFCD63996B080009160AA7967258317A61226E815BC9234AA4FEFE56C36DFE2C1E82E92FE9CC374CA888F7522FC679ED89E83487A16C7A7F5DB7B693E4B1D05F1A910AECBB8AE3740A248B970A5C233B09D5C66F9F812C25B2BABF1D9526C3EC79D9BA3E23BDC41B027B8363D5D5548354EE52BB93361CC64051B29FCB1CB8592F15C4FA0FB3C5CA3A00FA3BF2BBE68F20D98635EAAA89DC5990CBB20D2E693AEB08DA544F500";
//        QCArrayRef hex = QCArrayCreateWithHex(str, strlen(str));
//        if (QCObjectEqual(array, hex)) {
//            printf("eq");
//        }
//        QCRelease(hex);

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
