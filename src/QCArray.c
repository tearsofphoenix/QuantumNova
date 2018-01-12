//
// Created by Isaac on 2018/1/11.
//

#include "QCArray.h"
#include "QCArrayPrivate.h"
#include "QCDoubleArray.h"
#include "QCInt32Array.h"
#include "QCObject.h"
#include <fftw3.h>
#include <memory.h>
#include <math.h>

QCArrayRef QCArrayCreate(int count) {
    return QCArrayCreateWithDouble(NULL, count, false);
}

QCArrayRef QCArrayCreateWithDouble(const double *array, size_t count, bool needCopy) {
    return QCDoubleArrayCreate(array, count, needCopy);
}

QCArrayRef QCArrayCreateWithInt(const int *array, size_t count, bool needCopy) {
    return QCInt32ArrayCreate(array, count, needCopy);
}

QCArrayRef QCArrayCreateCopy(QCArrayRef array) {
    if (array) {
        return array->isa->copy(array);
    }
    return NULL;
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

QCArrayRef QCArrayExpPoly(QCArrayRef array, int64_t n) {
    size_t length = array->count;
    QCArrayRef y = QCArrayCreate(length);
    QCArraySetValueAt(y, 0, 1);
    QCArrayRef x = array;
    while (n > 1) {
        if (n % 2 == 0) {
            x = QCArraySquareSparsePoly(x, 1);
            n = n / 2;
        } else {
            // precision does not allow us to stay in FFT domain
            // hence, interchanging ifft(fft).
            QCArrayRef X = QCArrayFFT(x);
            QCArrayRef Y = QCArrayFFT(y);

            QCArrayRef temp = QCArrayComplexMultiply(X, Y);
            temp = QCArrayInverseFFT(temp);
            QCArrayRound(temp);
            QCArrayMod(temp, 2);
            y = temp;
            x = QCArraySquareSparsePoly(x, 1);
            n = (n - 1) / 2;
        }
    }
    QCArrayRef result = QCArrayMulPoly(x, y);
    QCArrayRound(result);
    QCArrayMod(result, 2);

    return result;
}
