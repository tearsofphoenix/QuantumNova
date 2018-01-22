//
// Created by Isaac on 2018/1/13.
//

#include "QNByteArray.h"
#include <tomcrypt.h>
#include <math.h>
#include <fftw3.h>

static void QNByteArrayEnumerator(QNArrayRef array, const void *func, const void *ctx);
static const void *QNByteArrayCopy(QNArrayRef array);
static QNArrayRef QNByteArrayAdd (QNArrayRef, QNArrayRef);
static QNArrayRef QNByteArrayMultiply (QNArrayRef , double );
static QNArrayRef QNByteArrayRound (QNArrayRef );
static QNArrayRef QNByteArrayMod (QNArrayRef , int mod);
static size_t QNByteArrayZeroCount (QNArrayRef );
static void QNByteArrayAddAt(QNArrayRef x, int index, double value);
static void QNByteArrayXORAt(QNArrayRef x, int index, int value);
static void QNByteArraySetAt(QNArrayRef x, int index, double value);
static double QNByteArrayGetAt(QNArrayRef x, int index);
static void QNByteArrayPrint(QNArrayRef );
static bool QNByteArrayEqual(QNArrayRef , QNArrayRef );
static double QNByteArrayMax(QNArrayRef array);

static QNArrayRef QNByteArrayGetNoZeroIndices(QNArrayRef array);
static QNArrayRef QNByteArraySHA256(QNArrayRef array);
static QNArrayRef QNByteArraySHA512(QNArrayRef array);
static bool QNByteArrayCompareRaw(QNArrayRef array, const void *expected, QNArrayDataType dataType);

static void QNByteArrayAppend(QNArrayRef array, QNArrayRef other);
static QNArrayRef QNByteArraySlice(QNArrayRef array, size_t start, size_t end);
static QNArrayRef QNByteArrayConvert(QNArrayRef array, QNArrayDataType type);
static QNArrayRef QNByteArrayPack(QNArrayRef array);
static bool QNByteArraySaveFile(QNArrayRef array, FILE *fp);
static size_t QNByteArrayGetSize(QNArrayRef );

static struct QNArrayClass kQNByteArrayClass = {
//        .base = kQNArrayClassRef,
        .name = "QNByteArray",
        .allocator = QNAllocator,
        .size = sizeof(struct QNArray),
        .deallocate = QNArrayDeallocate,
        .copy = QNByteArrayCopy,
        .enumerator = QNByteArrayEnumerator,
        .add = QNByteArrayAdd,
        .multiply = QNByteArrayMultiply,
        .round = QNByteArrayRound,
        .mod = QNByteArrayMod,
        .zero = QNByteArrayZeroCount,
        .addAt = QNByteArrayAddAt,
        .xorAt = QNByteArrayXORAt,
        .set = QNByteArraySetAt,
        .get = QNByteArrayGetAt,
        .print = QNByteArrayPrint,
        .equal = QNByteArrayEqual,
        .max = QNByteArrayMax,
        .nonzeroIndices = QNByteArrayGetNoZeroIndices,
        .sha256 = QNByteArraySHA256,
        .sha512 = QNByteArraySHA512,
        .compareRaw = QNByteArrayCompareRaw,
        .append = QNByteArrayAppend,
        .convert = QNByteArrayConvert,
        .slice = QNByteArraySlice,
        .pack = QNByteArrayPack,
        .saveFile = QNByteArraySaveFile,
        .getSize = QNByteArrayGetSize
};

const QNClassRef kQNByteArrayClassRef = &kQNByteArrayClass;

const void *QNByteArrayCreate(const void *initData, size_t count, bool needCopy) {
    QNArrayDataType type = QNDTByte;
    QNArrayRef array = QNAllocate(&kQNByteArrayClass);
    array->isa = kQNByteArrayClassRef;

    if (count > 0) {
        if (initData) {
            if (needCopy) {
                size_t size = 0;
                array->data = _QNMallocData(type, count, &size);
                memcpy(array->data, initData, size);
            } else {
                array->data = initData;
            }
            array->needfree = needCopy;
        } else {
            array->data = _QNMallocData(type, count, NULL);
            array->needfree = true;
        }
    } else {
        array->data = _QNMallocData(type, 1, NULL);
        array->needfree = true;
    }
    array->count = count;
    array->fft = false;
    array->datatype = type;
    return array;
}

QNArrayRef QNByteArrayCreateWithHex(const char *hexString, size_t length) {
    QNArrayDataType type = QNDTByte;
    QNArrayRef array = QNAllocate(&kQNByteArrayClass);
    array->isa = kQNByteArrayClassRef;
    QNByte *data = _QNMallocData(type, length / 2, NULL);
    char temp[] = {'0', 'x', '0', '0', '\0'};
    size_t j = 0;
    while(j < length) {
        temp[2] = hexString[j];
        temp[3] = hexString[j + 1];
        QNByte byte = strtoul(temp, NULL, 16);

        data[j / 2] = byte;
        j = j + 2;
    }

    array->datatype = type;
    array->data = data;
    array->count = length / 2;
    array->needfree = true;
    array->fft = false;
    return array;
}

QNArrayRef QNByteArrayFromFile(FILE *fp) {
    size_t count = 0;
    fread(&count, sizeof(size_t), 1, fp);
    QNByte *data = QNAllocator(sizeof(QNByte ) * count);
    fread(data, sizeof(QNByte), count, fp);
    QNArrayRef ref = QNArrayCreateWithByte(data, count, false);
    ref->needfree = true;

    return ref;
}

static size_t calcDecodeLength(const char* b64input) { //Calculates the length of a decoded string
    size_t len = strlen(b64input),
            padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len-1] == '=') //last char is =
        padding = 1;

    return (len * 3) / 4 - padding;
}

static int Base64Decode(char* b64message, unsigned char** buffer, size_t* length) { //Decodes a base64 encoded string

    size_t decodeLen = calcDecodeLength(b64message);
    *buffer = (unsigned char *)QNAllocator(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    base64_decode(b64message, strlen(b64message), *buffer, length);

    return (0); //success
}

QN_STRONG const char *QNEncodeBase64(QNByte *data, size_t length) {
    if (data && length > 0) {
        size_t size = (length * 4) / 3 + 4;
        char *result = QNAllocator(size * sizeof(char));
        base64_encode(data, length, result, &size);
        result[size] = '\0';
        return result;
    }
    return NULL;
}

QNArrayRef QNByteArrayCreateWithBase64(const char *base64String, size_t length) {
    QNArrayRef array = QNAllocate(&kQNByteArrayClass);
    array->isa = kQNByteArrayClassRef;

    size_t count = length;
    QNByte *data = NULL;
    Base64Decode(base64String, &data, &count);

    array->data = data;
    array->count = count;
    array->needfree = true;
    array->fft = false;
    array->datatype = QNDTByte;
    return array;
}

QNArrayRef QNByteArrayPKCS7Encode(QNArrayRef array) {
    size_t count = array->count;
    QNByte val = 16 - (count % 16);
    if (val == 16) {
        // no need to pad
        return QNArrayCreateCopy(array);
    } else {
        QNByte *str = QNAllocator(sizeof(QNByte) * val);
        for (size_t i = 0; i < val; ++i) {
            str[i] = val;
        }
        QNArrayRef pad = QNArrayCreateWithByte(str, val, false);
        pad->needfree = true;

        QNArrayRef result = QNArrayCreateCopy(array);
        QNArrayAppend(result, pad);
        QNRelease(pad);
        return result;
    }
}

// Remove the PKCS#7 padding from a text string
QNArrayRef QNByteArrayPKCS7Decode(QNArrayRef array) {
    size_t count = array->count;
    QNByte *data = array->data;
    int val = data[count - 1];
    if (val > 16) {
        // printf("Input is not padded or padding is corrupt!\n");
        return QNArrayCreateCopy(array);
    } else {
        return QNArraySlice(array, 0, count - val);
    }
}

static const void *QNByteArrayCopy(QNArrayRef array) {
    if (array) {
        return QNByteArrayCreate(array->data, array->count, true);
    }
    return NULL;
}


static void QNByteArrayEnumerator(QNArrayRef array, const void *func, const void *ctx) {
    QNByteArrayLoopFunc f = func;
    QNFOREACH(array, f(d[i], i, ctx), QNByte);
}

static QNArrayRef QNByteArrayAdd (QNArrayRef array, QNArrayRef y) {
    if (y->datatype == QNDTByte) {
        // both int
        QNByte *dy = y->data;
        QNFOREACH(array, d[i] += dy[i], QNByte);
    } else {
        double *dy = y->data;
        QNFOREACH(array, d[i] += dy[i], QNByte);
    }
    return array;
}

static QNArrayRef QNByteArrayMultiply (QNArrayRef array, double mul) {
    QNByte m = (QNByte)mul;
    QNFOREACH(array, d[i] *= m, QNByte);
    return array;
}
static QNArrayRef QNByteArrayRound (QNArrayRef array) {
    // no need to round
    return array;
}
static QNArrayRef QNByteArrayMod (QNArrayRef array, int mod) {
    QNFOREACH(array, d[i] = (QNByte)(d[i] % mod), QNByte);
    return array;
}

static size_t QNByteArrayZeroCount (QNArrayRef array) {
    size_t total = 0;
    QNFOREACH(array, if (d[i] == 0){ ++total; }, QNByte);
    return total;
}

QNARRAYIMP(QNByteArray, QNByte)

static void QNByteArrayPrint(QNArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x count: %d>[ ", array->isa->name, array, array->count);

        QNFOREACH(array, printf("%02x", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, QNByte);

        printf(" ]\n");
    }
}

static bool QNByteArrayEqual(QNArrayRef array, QNArrayRef y) {
    if (y->datatype == QNDTByte) {
        return memcmp(array->data, y->data, array->count * sizeof(QNByte)) == 0;
    } else {
        double *dy = y->data;
        QNFOREACH(array, if (d[i] != (int)dy[i]) { return false; }, QNByte);
        return true;
    }
}

static double QNByteArrayMax(QNArrayRef array) {
    int max = 0;
    QNFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, QNByte);
    return max;
}

static QNArrayRef QNByteArrayGetNoZeroIndices(QNArrayRef array) {
    if (array) {
        size_t count = array->count;
        QNByte *x = array->data;
        QNByte *indices = _QNMallocData(QNDTByte, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if (x[i] != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        QNArrayRef ref = QNArrayCreateWithInt(indices, idx, false);
        ref->needfree = true;
        return ref;
    }
    return NULL;
}


static void QNSHA256(const void *data, size_t size, QNByte *buf) {
    hash_state    ctx;

    sha256_init(&ctx);
    sha256_process(&ctx, data, size);
    sha256_done(&ctx, buf);
}

static QNArrayRef QNByteArraySHA256(QNArrayRef array) {
    if (array) {
#define SHA256_BLOCK_SIZE 32
        QNByte buf[SHA256_BLOCK_SIZE];
        QNSHA256(array->data, sizeof(QNByte) * array->count, buf);
        return QNByteArrayCreate(buf, SHA256_BLOCK_SIZE, true);
    }
    return NULL;
}

static QNArrayRef QNByteArraySHA512(QNArrayRef array) {
#define SHA512_HASH_SIZE 64
    size_t len = array->count;
    QNByte tmp[SHA512_HASH_SIZE];
    hash_state ctx;
    sha512_init(&ctx);
    sha512_process(&ctx, array->data, len);
    sha512_done(&ctx, tmp);
    return QNByteArrayCreate(tmp, SHA512_HASH_SIZE, true);
}

QNARRAYCOMPARE(_arrayCompareInt, QNByte, int, "not equal: %d %d %d\n")

QNARRAYCOMPARE(_arrayCompareDouble, QNByte, double, "not equal: %d %d %f\n")

static bool QNByteArrayCompareRaw(QNArrayRef array, const void *expected, QNArrayDataType dataType) {
    switch (dataType) {
        case QNDTInt: {
            return _arrayCompareInt(array->data, expected, array->count);
        }
        case QNDTDouble: {
            return _arrayCompareDouble(array->data, expected, array->count);
        }
        default: {
            return memcmp(array->data, expected, sizeof(QNByte) * array->count) == 0;
        }

    }
    return false;
}

static int byteArrayToInt(const QNByte * b) {
    return (b[0] << 24)
           + ((b[1] & 0xFF) << 16)
           + ((b[2] & 0xFF) << 8)
           + (b[3] & 0xFF);
}

static QNArrayRef QNByteArrayConvert(QNArrayRef array, QNArrayDataType type) {
    if (array) {
        switch (type) {
            case QNDTInt: {
                size_t count = (size_t)ceilf(array->count / 4);
                int *data = array->isa->allocator(count * sizeof(int));
                for (size_t i = 0; i < array->count; i = i + 4) {
                    data[i / 4] = byteArrayToInt(array->data + i);
                }
                QNArrayRef result = QNArrayCreateWithInt(data, count, false);
                result->needfree = true;
                return result;
            }
            case QNDTDouble: {
                size_t count = (size_t)ceilf(array->count / 4);
                double *data = array->isa->allocator(count * sizeof(int));
                for (size_t i = 0; i < array->count; i = i + 4) {
                    data[i / 4] = (double)byteArrayToInt(array->data + i);
                }
                QNArrayRef result = QNArrayCreateWithDouble(data, count, false);
                result->needfree = true;
                return result;
            }
            case QNDTByte: {
                return QNByteArrayCreate(array->data, array->count, true);
            }
            default: {
                return NULL;
            }
        }
    }
    return NULL;
}
