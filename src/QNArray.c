//
// Created by Isaac on 2018/1/11.
//

#include "QNArray.h"
#include "QNArrayPrivate.h"
#include "QNDoubleArray.h"
#include "QNInt32Array.h"
#include "QNByteArray.h"
#include <fftw3.h>
#include <math.h>

QNArrayRef QNArrayCreate(size_t count) {
    return QNArrayCreateWithDouble(NULL, count, true);
}

QNArrayRef QNArrayCreateWithDouble(const double *array, size_t count, bool needCopy) {
    return QNDoubleArrayCreate(array, count, needCopy);
}

QNArrayRef QNArrayCreateWithInt(const int *array, size_t count, bool needCopy) {
    return QNInt32ArrayCreate(array, count, needCopy);
}

QNArrayRef QNArrayCreateWithHex(const char *hexString, size_t length) {
    return QNByteArrayCreateWithHex(hexString, length);
}

QNArrayRef QNArrayPKCS7Encode(QNArrayRef array) {
    return QNByteArrayPKCS7Encode(array);
}

QNArrayRef QNArrayPKCS7Decode(QNArrayRef array) {
    return QNByteArrayPKCS7Decode(array);
}

QNArrayRef QNArrayFromFile(FILE *fp) {
    QNArrayDataType type;
    fread(&type, sizeof(QNArrayDataType), 1, fp);
    switch (type) {
        case QNDTInt: {
            return QNInt32ArrayFromFile(fp);
        }
        case QNDTByte: {
            return QNByteArrayFromFile(fp);
        }
        default: {
            return QNDoubleArrayFromFile(fp);
        }
    }
    return NULL;
}

bool QNArraySaveToFile(QNArrayRef array, FILE *fp) {
    return ((QNArrayClassRef)array->isa)->saveFile(array, fp);
}

QNArrayRef QNArrayCreateCopy(QNArrayRef array) {
    if (array) {
        return array->isa->copy(array);
    }
    return NULL;
}

QNArrayRef QNArraySHA256(QNArrayRef array) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->sha256(array);
    }
    return NULL;
}

QNArrayRef QNArraySHA512(QNArrayRef array) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->sha512(array);
    }
    return NULL;
}

QNArrayRef QNArrayPack(QNArrayRef array) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->pack(array);
    }
    return NULL;
}

void QNArrayAppend(QNArrayRef array, QNArrayRef other) {
    if (array) {
        ((QNArrayClassRef)array->isa)->append(array, other);
    }
}

QNArrayRef QNArraySlice(QNArrayRef array, size_t start, size_t end) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->slice(array, start, end);
    }
    return NULL;
}

QNArrayRef QNArrayConvert(QNArrayRef array, QNArrayDataType type) {
    if (array) {
        return ((QNArrayClassRef)array->isa)->convert(array, type);
    }
    return NULL;
}

QNArrayRef QNArrayCreateWithByte(const QNByte *array, size_t count, bool needCopy) {
    return QNByteArrayCreate(array, count, needCopy);
}

QNArrayRef QNArrayCreateWithBase64(const char *base64String, size_t length) {
    return QNByteArrayCreateWithBase64(base64String, length);
}

size_t QNArrayGetSize(QNArrayRef array) {
    return ((QNArrayClassRef)array->isa)->getSize(array);
}

QNArrayRef QNArrayFFT(QNArrayRef array) {
    if (array) {
        size_t count = array->count;
        double *out = fftw_malloc((count + 2) * sizeof(double));

        fftw_plan plan = fftw_plan_dft_r2c_1d((int)count, array->data, (void *)out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        QNArrayRef result = QNArrayCreateWithDouble(out, count + 2, true);
        result->fft = true;
        return result;
    }
    return NULL;
}

QNArrayRef QNArrayInverseFFT(QNArrayRef array) {
    if (array) {
        size_t count = array->count;
        size_t realCount = count - 2;
        double *out = fftw_malloc(realCount * sizeof(double));

        QNArrayMultiply(array, 1.0 / realCount);
        fftw_plan plan = fftw_plan_dft_c2r_1d((int)realCount, array->data, out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        QNArrayRef result = QNArrayCreateWithDouble(out, realCount, true);
        return result;
    }
    return NULL;
}

QNArrayRef QNArrayComplexMultiply(QNArrayRef xArray, QNArrayRef yArray) {
    if (xArray && yArray) {
        return ((QNArrayClassRef)xArray->isa)->complexMultiply(xArray, yArray);
    }
    return NULL;
}

QNArrayRef QNArraySquareSparsePoly(QNArrayRef array, int times) {
    if (array) {
        times = 1;

        size_t count = array->count;
        QNArrayRef indices = QNArrayGetNoZeroIndices(array);

        size_t mod = count;
        QNArrayRef result = QNArrayCreate(mod);
        double *x = result->data;
        size_t mul = (int) pow(2, times) % mod;
        QNArrayMultiply(indices, mul);

        int *d = indices->data;
        for (int i = 0; i < indices->count; ++i) {
            int index = d[i];
            size_t idx = index % mod;
            x[idx] = (int) x[idx] ^ 1;
        }

        QNRelease(indices);

        return result;
    }
    return NULL;
}

QNArrayRef QNArrayMulPoly(QNArrayRef x, QNArrayRef y) {
    size_t count = x->count;
    QNArrayRef fx = QNArrayFFT(x);
    QNArrayRef fy = QNArrayFFT(y);
    QNArrayRef mul = QNArrayComplexMultiply(fx, fy);
    QNArrayRef result = QNArrayInverseFFT(mul);
    QNArraySetCount(result, count);

    QNArrayRound(result);
    QNArrayMod(result, 2);

    QNRelease(fx);
    QNRelease(fy);
    QNRelease(mul);

    return result;
}

QNArrayRef QNArrayExpPoly(QNArrayRef array, BIGNUM *n) {
    size_t length = array->count;
    QNArrayRef y = QNArrayCreate(length);
    QNArraySetValueAt(y, 0, 1);
    QNArrayRef x = QNArrayCreateCopy(array);

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *one = NULL;
    BIGNUM *two = NULL;
    BIGNUM *mod = BN_new();
    BN_dec2bn(&one, "1");
    BN_dec2bn(&two, "2");

    while (BN_cmp(n, one) == 1) {
        BN_mod(mod, n, two, ctx);
        QNArrayRef tx = x;

        if (BN_is_zero(mod)) {
            x = QNArraySquareSparsePoly(tx, 1);
            QNRelease(tx);

            BN_div(n, NULL, n, two, ctx);
        } else {
            // precision does not allow us to stay in FFT domain
            // hence, interchanging ifft(fft).
            QNArrayRef X = QNArrayFFT(x);
            QNArrayRef Y = QNArrayFFT(y);

            QNArrayRef t = QNArrayComplexMultiply(X, Y);
            QNArrayRef temp = QNArrayInverseFFT(t);
            QNArrayRound(temp);
            QNArrayMod(temp, 2);

            QNRelease(X);
            QNRelease(Y);
            QNRelease(t);
            QNRelease(y);
            y = temp;
            x = QNArraySquareSparsePoly(x, 1);
            QNRelease(tx);

            BN_sub(n, n, one);
            BN_div(n, NULL, n, two, ctx);
        }
    }

    BN_free(one);
    BN_free(two);
    BN_free(mod);
    BN_CTX_free(ctx);

    QNArrayRef result = QNArrayMulPoly(x, y);
    QNArrayRound(result);
    QNArrayMod(result, 2);

    QNRelease(x);
    QNRelease(y);

    return result;
}
