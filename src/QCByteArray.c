//
// Created by Isaac on 2018/1/13.
//

#include "QCByteArray.h"
#include <tomcrypt.h>
#include <math.h>
#include <fftw3.h>

static void QCByteArrayEnumerator(QCArrayRef array, const void *func, const void *ctx);
static const void *QCByteArrayCopy(QCArrayRef array);
static QCArrayRef QCByteArrayAdd (QCArrayRef, QCArrayRef);
static QCArrayRef QCByteArrayMultiply (QCArrayRef , double );
static QCArrayRef QCByteArrayRound (QCArrayRef );
static QCArrayRef QCByteArrayMod (QCArrayRef , int mod);
static size_t QCByteArrayZeroCount (QCArrayRef );
static void QCByteArrayAddAt(QCArrayRef x, int index, double value);
static void QCByteArrayXORAt(QCArrayRef x, int index, int value);
static void QCByteArraySetAt(QCArrayRef x, int index, double value);
static double QCByteArrayGetAt(QCArrayRef x, int index);
static void QCByteArrayPrint(QCArrayRef );
static bool QCByteArrayEqual(QCArrayRef , QCArrayRef );
static double QCByteArrayMax(QCArrayRef array);

static QCArrayRef QCByteArrayGetNoZeroIndices(QCArrayRef array);
static QCArrayRef QCByteArraySHA256(QCArrayRef array);
static QCArrayRef QCByteArraySHA512(QCArrayRef array);
static bool QCByteArrayCompareRaw(QCArrayRef array, const void *expected, QCArrayDataType dataType);

static void QCByteArrayAppend(QCArrayRef array, QCArrayRef other);
static QCArrayRef QCByteArraySlice(QCArrayRef array, size_t start, size_t end);
static QCArrayRef QCByteArrayConvert(QCArrayRef array, QCArrayDataType type);
static QCArrayRef QCByteArrayPack(QCArrayRef array);

static struct QCArrayClass kQCByteArrayClass = {
//        .base = kQCArrayClassRef,
        .name = "QCByteArray",
        .allocator = QCAllocator,
        .size = sizeof(struct QCArray),
        .deallocate = QCArrayDeallocate,
        .copy = QCByteArrayCopy,
        .enumerator = QCByteArrayEnumerator,
        .add = QCByteArrayAdd,
        .multiply = QCByteArrayMultiply,
        .round = QCByteArrayRound,
        .mod = QCByteArrayMod,
        .zero = QCByteArrayZeroCount,
        .addAt = QCByteArrayAddAt,
        .xorAt = QCByteArrayXORAt,
        .set = QCByteArraySetAt,
        .get = QCByteArrayGetAt,
        .print = QCByteArrayPrint,
        .equal = QCByteArrayEqual,
        .max = QCByteArrayMax,
        .nonzeroIndices = QCByteArrayGetNoZeroIndices,
        .sha256 = QCByteArraySHA256,
        .sha512 = QCByteArraySHA512,
        .compareRaw = QCByteArrayCompareRaw,
        .append = QCByteArrayAppend,
        .convert = QCByteArrayConvert,
        .slice = QCByteArraySlice,
        .pack = QCByteArrayPack
};

const QCClassRef kQCByteArrayClassRef = &kQCByteArrayClass;

const void *QCByteArrayCreate(const void *initData, size_t count, bool needCopy) {
    QCArrayDataType type = QCDTByte;
    QCArrayRef array = QCAllocate(&kQCByteArrayClass);
    array->isa = kQCByteArrayClassRef;

    if (count > 0) {
        if (initData) {
            if (needCopy) {
                size_t size = 0;
                array->data = _QCMallocData(type, count, &size);
                memcpy(array->data, initData, size);
            } else {
                array->data = initData;
            }
            array->needfree = needCopy;
        } else {
            array->data = _QCMallocData(type, count, NULL);
            array->needfree = true;
        }
    } else {
        array->data = _QCMallocData(type, 1, NULL);
        array->needfree = true;
    }
    array->count = count;
    array->fft = false;
    array->datatype = type;
    return array;
}

QCArrayRef QCByteArrayCreateWithHex(const char *hexString, size_t length) {
    QCArrayDataType type = QCDTByte;
    QCArrayRef array = QCAllocate(&kQCByteArrayClass);
    array->isa = kQCByteArrayClassRef;
    QCByte *data = _QCMallocData(type, length / 2, NULL);
    char temp[] = {'0', 'x', '0', '0', '\0'};
    size_t j = 0;
    while(j < length) {
        temp[2] = hexString[j];
        temp[3] = hexString[j + 1];
        QCByte byte = strtoul(temp, NULL, 16);

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
    *buffer = (unsigned char *)QCAllocator(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    base64_decode(b64message, strlen(b64message), *buffer, length);

    return (0); //success
}

QC_STRONG const char *QCEncodeBase64(QCByte *data, size_t length) {
    if (data && length > 0) {
        size_t size = (length * 4) / 3 + 4;
        char *result = QCAllocator(size * sizeof(char));
        base64_encode(data, length, result, &size);
        result[size] = '\0';
        return result;
    }
    return NULL;
}

QCArrayRef QCByteArrayCreateWithBase64(const char *base64String, size_t length) {
    QCArrayRef array = QCAllocate(&kQCByteArrayClass);
    array->isa = kQCByteArrayClassRef;

    size_t count = length;
    QCByte *data = NULL;
    Base64Decode(base64String, &data, &count);

    array->data = data;
    array->count = count;
    array->needfree = true;
    array->fft = false;
    array->datatype = QCDTByte;
    return array;
}

QCArrayRef QCByteArrayPKCS7Encode(QCArrayRef array) {
    size_t count = array->count;
    QCByte val = 16 - (count % 16);
    if (val == 16) {
        // no need to pad
        return QCArrayCreateCopy(array);
    } else {
        QCByte *str = QCAllocator(sizeof(QCByte) * val);
        for (size_t i = 0; i < val; ++i) {
            str[i] = val;
        }
        QCArrayRef pad = QCArrayCreateWithByte(str, val, false);
        pad->needfree = true;

        QCArrayRef result = QCArrayCreateCopy(array);
        QCArrayAppend(result, pad);
        QCRelease(pad);
        return result;
    }
}

// Remove the PKCS#7 padding from a text string
QCArrayRef QCByteArrayPKCS7Decode(QCArrayRef array) {
    size_t count = array->count;
    QCByte *data = array->data;
    int val = data[count - 1];
    if (val > 16) {
        // printf("Input is not padded or padding is corrupt!\n");
        return QCArrayCreateCopy(array);
    } else {
        return QCArraySlice(array, 0, count - val);
    }
}

static const void *QCByteArrayCopy(QCArrayRef array) {
    if (array) {
        return QCByteArrayCreate(array->data, array->count, true);
    }
    return NULL;
}


static void QCByteArrayEnumerator(QCArrayRef array, const void *func, const void *ctx) {
    QCByteArrayLoopFunc f = func;
    QCFOREACH(array, f(d[i], i, ctx), QCByte);
}

static QCArrayRef QCByteArrayAdd (QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTByte) {
        // both int
        QCByte *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], QCByte);
    } else {
        double *dy = y->data;
        QCFOREACH(array, d[i] += dy[i], QCByte);
    }
    return array;
}

static QCArrayRef QCByteArrayMultiply (QCArrayRef array, double mul) {
    QCByte m = (QCByte)mul;
    QCFOREACH(array, d[i] *= m, QCByte);
    return array;
}
static QCArrayRef QCByteArrayRound (QCArrayRef array) {
    // no need to round
    return array;
}
static QCArrayRef QCByteArrayMod (QCArrayRef array, int mod) {
    QCFOREACH(array, d[i] = (QCByte)(d[i] % mod), QCByte);
    return array;
}

static size_t QCByteArrayZeroCount (QCArrayRef array) {
    size_t total = 0;
    QCFOREACH(array, if (d[i] == 0){ ++total; }, QCByte);
    return total;
}

QCARRAYIMP(QCByteArray, QCByte)

static void QCByteArrayPrint(QCArrayRef array) {
    if (array) {
        int padding = 25;
        printf("\n<%s 0x%x count: %d>[ ", array->isa->name, array, array->count);

        QCFOREACH(array, printf("%02x", d[i]); if (i % padding == 0 && i > 0) { printf("\n"); }, QCByte);

        printf(" ]\n");
    }
}

static bool QCByteArrayEqual(QCArrayRef array, QCArrayRef y) {
    if (y->datatype == QCDTByte) {
        return memcmp(array->data, y->data, array->count * sizeof(QCByte)) == 0;
    } else {
        double *dy = y->data;
        QCFOREACH(array, if (d[i] != (int)dy[i]) { return false; }, QCByte);
        return true;
    }
}

static double QCByteArrayMax(QCArrayRef array) {
    int max = 0;
    QCFOREACH(array, if (max < d[i]) {
        max = d[i];
    }, QCByte);
    return max;
}

static QCArrayRef QCByteArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        size_t count = array->count;
        QCByte *x = array->data;
        QCByte *indices = _QCMallocData(QCDTByte, count, NULL);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if (x[i] != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        QCArrayRef ref = QCArrayCreateWithInt(indices, idx, false);
        ref->needfree = true;
        return ref;
    }
    return NULL;
}


static void QCSHA256(const void *data, size_t size, QCByte *buf) {
    hash_state    ctx;

    sha256_init(&ctx);
    sha256_process(&ctx, data, size);
    sha256_done(&ctx, buf);
}

static QCArrayRef QCByteArraySHA256(QCArrayRef array) {
    if (array) {
#define SHA256_BLOCK_SIZE 32
        QCByte buf[SHA256_BLOCK_SIZE];
        QCSHA256(array->data, sizeof(QCByte) * array->count, buf);
        return QCByteArrayCreate(buf, SHA256_BLOCK_SIZE, true);
    }
    return NULL;
}

static QCArrayRef QCByteArraySHA512(QCArrayRef array) {
#define SHA512_HASH_SIZE 64
    size_t len = array->count;
    QCByte tmp[SHA512_HASH_SIZE];
    hash_state ctx;
    sha512_init(&ctx);
    sha512_process(&ctx, array->data, len);
    sha512_done(&ctx, tmp);
    return QCByteArrayCreate(tmp, SHA512_HASH_SIZE, true);
}

QCARRAYCOMPARE(_arrayCompareInt, QCByte, int, "not equal: %d %d %d\n")

QCARRAYCOMPARE(_arrayCompareDouble, QCByte, double, "not equal: %d %d %f\n")

static bool QCByteArrayCompareRaw(QCArrayRef array, const void *expected, QCArrayDataType dataType) {
    switch (dataType) {
        case QCDTInt: {
            return _arrayCompareInt(array->data, expected, array->count);
        }
        case QCDTDouble: {
            return _arrayCompareDouble(array->data, expected, array->count);
        }
        default: {
            return memcmp(array->data, expected, sizeof(QCByte) * array->count) == 0;
        }

    }
    return false;
}

static int byteArrayToInt(const QCByte * b) {
    return (b[0] << 24)
           + ((b[1] & 0xFF) << 16)
           + ((b[2] & 0xFF) << 8)
           + (b[3] & 0xFF);
}

static QCArrayRef QCByteArrayConvert(QCArrayRef array, QCArrayDataType type) {
    if (array) {
        switch (type) {
            case QCDTInt: {
                size_t count = (size_t)ceilf(array->count / 4);
                int *data = array->isa->allocator(count * sizeof(int));
                for (size_t i = 0; i < array->count; i = i + 4) {
                    data[i / 4] = byteArrayToInt(array->data + i);
                }
                QCArrayRef result = QCArrayCreateWithInt(data, count, false);
                result->needfree = true;
                return result;
            }
            case QCDTDouble: {
                size_t count = (size_t)ceilf(array->count / 4);
                double *data = array->isa->allocator(count * sizeof(int));
                for (size_t i = 0; i < array->count; i = i + 4) {
                    data[i / 4] = (double)byteArrayToInt(array->data + i);
                }
                QCArrayRef result = QCArrayCreateWithDouble(data, count, false);
                result->needfree = true;
                return result;
            }
            case QCDTByte: {
                return QCByteArrayCreate(array->data, array->count, true);
            }
            default: {
                return NULL;
            }
        }
    }
    return NULL;
}
