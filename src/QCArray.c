//
// Created by Isaac on 2018/1/11.
//

#include "QCArray.h"
#include "QCArrayPrivate.h"
#include "QCDoubleArray.h"
#include "QCInt32Array.h"
#include "QCByteArray.h"
#include <fftw3.h>
#include <math.h>

QCArrayRef QCArrayCreate(int count) {
    return QCArrayCreateWithDouble(NULL, count, true);
}

QCArrayRef QCArrayCreateWithDouble(const double *array, size_t count, bool needCopy) {
    return QCDoubleArrayCreate(array, count, needCopy);
}

QCArrayRef QCArrayCreateWithInt(const int *array, size_t count, bool needCopy) {
    return QCInt32ArrayCreate(array, count, needCopy);
}

QCArrayRef QCArrayCreateWithHex(const char *hexString, size_t length) {
    return QCByteArrayCreateWithHex(hexString, length);
}

QCArrayRef QCArrayPKCS7Encode(QCArrayRef array) {
    return QCByteArrayPKCS7Encode(array);
}

QCArrayRef QCArrayPKCS7Decode(QCArrayRef array) {
    return QCByteArrayPKCS7Decode(array);
}

QCArrayRef QCArrayFromFile(FILE *fp) {
    QCArrayDataType type;
    fread(&type, sizeof(QCArrayDataType), 1, fp);
    switch (type) {
        case QCDTInt: {
            return QCInt32ArrayFromFile(fp);
        }
        case QCDTByte: {
            return QCByteArrayFromFile(fp);
        }
        default: {
            return QCDoubleArrayFromFile(fp);
        }
    }
    return NULL;
}

bool QCArraySaveToFile(QCArrayRef array, FILE *fp) {
    return ((QCArrayClassRef)array->isa)->saveFile(array, fp);
}

QCArrayRef QCArrayCreateCopy(QCArrayRef array) {
    if (array) {
        return array->isa->copy(array);
    }
    return NULL;
}

QCArrayRef QCArraySHA256(QCArrayRef array) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->sha256(array);
    }
    return NULL;
}

QCArrayRef QCArraySHA512(QCArrayRef array) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->sha512(array);
    }
    return NULL;
}

QCArrayRef QCArrayPack(QCArrayRef array) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->pack(array);
    }
    return NULL;
}

void QCArrayAppend(QCArrayRef array, QCArrayRef other) {
    if (array) {
        ((QCArrayClassRef)array->isa)->append(array, other);
    }
}

QCArrayRef QCArraySlice(QCArrayRef array, size_t start, size_t end) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->slice(array, start, end);
    }
    return NULL;
}

QCArrayRef QCArrayConvert(QCArrayRef array, QCArrayDataType type) {
    if (array) {
        return ((QCArrayClassRef)array->isa)->convert(array, type);
    }
    return NULL;
}

QCArrayRef QCArrayCreateWithByte(const QCByte *array, size_t count, bool needCopy) {
    return QCByteArrayCreate(array, count, needCopy);
}

QCArrayRef QCArrayCreateWithBase64(const char *base64String, size_t length) {
    return QCByteArrayCreateWithBase64(base64String, length);
}

QCArrayRef QCArrayFFT(QCArrayRef array) {
    if (array) {
        size_t count = array->count;
        double *out = fftw_malloc((count + 2) * sizeof(double));

        fftw_plan plan = fftw_plan_dft_r2c_1d(count, array->data, (void *)out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        QCArrayRef result = QCArrayCreateWithDouble(out, count + 2, true);
        result->fft = true;
        return result;
    }
    return NULL;
}

QCArrayRef QCArrayInverseFFT(QCArrayRef array) {
    if (array) {
        size_t count = array->count;
        size_t realCount = count - 2;
        double *out = fftw_malloc(realCount * sizeof(double));

        QCArrayMultiply(array, 1.0 / realCount);
        fftw_plan plan = fftw_plan_dft_c2r_1d(realCount, array->data, out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        QCArrayRef result = QCArrayCreateWithDouble(out, realCount, true);
        return result;
    }
    return NULL;
}

QCArrayRef QCArrayComplexMultiply(QCArrayRef xArray, QCArrayRef yArray) {
    if (xArray && yArray) {
        return ((QCArrayClassRef)xArray->isa)->complexMultiply(xArray, yArray);
    }
    return NULL;
}

QCArrayRef QCArraySquareSparsePoly(QCArrayRef array, int times) {
    if (array) {
        times = 1;

        size_t count = array->count;
        QCArrayRef indices = QCArrayGetNoZeroIndices(array);

        size_t mod = count;
        QCArrayRef result = QCArrayCreate(mod);
        double *x = result->data;
        size_t mul = (int) pow(2, times) % mod;
        QCArrayMultiply(indices, mul);

        int *d = indices->data;
        for (int i = 0; i < indices->count; ++i) {
            int index = d[i];
            size_t idx = index % mod;
            x[idx] = (int) x[idx] ^ 1;
        }

        QCRelease(indices);

        return result;
    }
    return NULL;
}

QCArrayRef QCArrayMulPoly(QCArrayRef x, QCArrayRef y) {
    size_t count = x->count;
    QCArrayRef fx = QCArrayFFT(x);
    QCArrayRef fy = QCArrayFFT(y);
    QCArrayRef mul = QCArrayComplexMultiply(fx, fy);
    QCArrayRef result = QCArrayInverseFFT(mul);
    QCArraySetCount(result, count);

    QCArrayRound(result);
    QCArrayMod(result, 2);

    QCRelease(fx);
    QCRelease(fy);
    QCRelease(mul);

    return result;
}

QCArrayRef QCArrayExpPoly(QCArrayRef array, BIGNUM *n) {
    size_t length = array->count;
    QCArrayRef y = QCArrayCreate(length);
    QCArraySetValueAt(y, 0, 1);
    QCArrayRef x = QCArrayCreateCopy(array);

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *one = NULL;
    BIGNUM *two = NULL;
    BIGNUM *mod = BN_new();
    BN_dec2bn(&one, "1");
    BN_dec2bn(&two, "2");

    while (BN_cmp(n, one) == 1) {
        BN_mod(mod, n, two, ctx);
        QCArrayRef tx = x;

        if (BN_is_zero(mod)) {
            x = QCArraySquareSparsePoly(tx, 1);
            QCRelease(tx);

            BN_div(n, NULL, n, two, ctx);
        } else {
            // precision does not allow us to stay in FFT domain
            // hence, interchanging ifft(fft).
            QCArrayRef X = QCArrayFFT(x);
            QCArrayRef Y = QCArrayFFT(y);

            QCArrayRef t = QCArrayComplexMultiply(X, Y);
            QCArrayRef temp = QCArrayInverseFFT(t);
            QCArrayRound(temp);
            QCArrayMod(temp, 2);

            QCRelease(X);
            QCRelease(Y);
            QCRelease(t);
            QCRelease(y);
            y = temp;
            x = QCArraySquareSparsePoly(x, 1);
            QCRelease(tx);

            BN_sub(n, n, one);
            BN_div(n, NULL, n, two, ctx);
        }
    }

    BN_free(one);
    BN_free(two);
    BN_free(mod);
    BN_CTX_free(ctx);

    QCArrayRef result = QCArrayMulPoly(x, y);
    QCArrayRound(result);
    QCArrayMod(result, 2);

    QCRelease(x);
    QCRelease(y);

    return result;
}
