//
// Created by Isaac on 2018/1/11.
//

#include "QCArray.h"
#include "QCArrayPrivate.h"
#include <fftw3.h>
#include <memory.h>
#include <math.h>

QCArrayRef QCArrayCreate(int count) {
    return QCArrayCreateWithType(QCDTDouble, count);
}

QCArrayRef QCArrayCreateWithType(QCArrayDataType type, int count) {
    QCArrayRef array = fftw_malloc(sizeof(*array));
    array->data = _QCMallocData(type, count, NULL);
    array->count = count;
    array->fft = false;
    array->needfree = true;
    array->datatype = type;
    return array;
}

QCArrayRef QCArrayCreateCopy(QCArrayRef array) {
    QCArrayRef ref = fftw_malloc(sizeof(*array));
    size_t size = 0;
    ref->data = _QCMallocData((QCArrayDataType)array->datatype, array->count, &size);
    memcpy(ref->data, array->data, size);
    ref->count = array->count;
    ref->fft = array->fft;
    ref->needfree = true;
    ref->datatype = array->datatype;
    return array;
}

QCArrayRef QCArrayCreateFrom(const double *x, int count) {
    QCArrayRef array = QCArrayCreate(count);
    memcpy(array->data, x, sizeof(double) * count);
    return array;
}

QCArrayRef QCArrayCreateNoCopy(void *x, int count, bool needfree) {
    QCArrayRef array = fftw_malloc(sizeof(*array));
    array->data = x;
    array->count = count;
    array->fft = false;
    array->needfree = needfree;
    array->datatype = QCDTDouble;
    return array;
}

void QCArrayForeach(QCArrayRef array, QCArrayEnumerator func, const void *ctx) {
    if (array && func) {
        QCARRAYEACH(array, func(d[i], i, ctx), func(d[i], i, ctx));
    }
}

QCArrayRef QCArrayFFT(QCArrayRef array) {
    if (array) {
        int count = array->count;
        double *out = fftw_malloc((count + 2) * sizeof(double));

        fftw_plan plan = fftw_plan_dft_r2c_1d(count, array->data, (void *)out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        QCArrayRef result = QCArrayCreateNoCopy(out, count + 2, true);
        result->fft = true;
        return result;
    }
    return NULL;
}

void QCArrayScale(QCArrayRef array, double scale) {
    if (array && scale != 0) {
        int count = array->count;
        double *x = array->data;
        int i = 0;
        while (i < count) {
            x[i] *= scale;
            i++;
        }
    }
}

QCArrayRef QCArrayInverseFFT(QCArrayRef array) {
    if (array) {
        int count = array->count;
        int realCount = count - 2;
        double *out = fftw_malloc(realCount * sizeof(double));

        QCArrayScale(array, 1.0 / realCount);
        fftw_plan plan = fftw_plan_dft_c2r_1d(realCount, array->data, out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        QCArrayRef result = QCArrayCreateNoCopy(out, realCount, true);
        return result;
    }
    return NULL;
}

QCArrayRef QCArrayComplexMultiply(QCArrayRef xArray, QCArrayRef yArray) {
    if (xArray && yArray) {
        int count = xArray->count;
        double *x = xArray->data;
        double *y = yArray->data;
        double *out = fftw_malloc(count * sizeof(double));
        for (int i = 0; i < count; i = i + 2) {
            double a = x[i];
            double b = x[i + 1];
            double c = y[i];
            double d = y[i + 1];
            out[i] = a * c - b * d;
            out[i + 1] = a * d + b * c;
        }
        if (count % 2 == 1) {
            out[count - 1] = x[count - 1] * y[count - 1];
        }
        return QCArrayCreateNoCopy(out, count, true);
    }
    return NULL;
}

void QCArrayAddArray(QCArrayRef x, QCArrayRef y) {
    if (x && y) {
        QCARRAYEACH(x, d[i] += ((int *)y->data)[i], d[i] += ((double *)y->data)[i]);
    }
}

void QCArrayMultiply(QCArrayRef array, double mul) {
    if (array) {
        QCARRAYEACH(array, d[i] *= mul, d[i] *= mul);
    }
}

void QCArrayRound(QCArrayRef array) {
    if (array) {
        QCARRAYEACH(array, d[i] = (int)round(d[i]), d[i] = round(d[i]));
    }
}

double QCArrayMax(QCArrayRef array) {
    if (array) {
        int count = array->count;
        switch (array->datatype) {
            case QCDTInt: {
                int *d = array->data;
                int value = 0;
                for (int i = 0; i < count; ++i) {
                    if (value < d[i]) {
                        value = d[i];
                    }
                }
                return value;
            }
            default: {
                double *d = array->data;
                double value = 0;
                for (int i = 0; i < count; ++i) {
                    if (value < d[i]) {
                        value = d[i];
                    }
                }
                return value;
            }
        }
    }
    return 0;
}

void QCArrayAddAt(QCArrayRef array, int index, double value) {
    if (array && index < array->count) {
        QCARRAYEACH(array, d[index] += (int)value, d[index] += value);
    }
}

void QCArrayMod(QCArrayRef array, int mod) {
    if (array) {
        QCARRAYEACH(array, d[i] = d[i] % mod, d[i] = (int) d[i] % mod);
    }
}

QCArrayRef QCArrayGetRealParts(QCArrayRef complexArray) {
    if (complexArray) {
        int count = complexArray->count;
        if (complexArray->fft) {
            count -= 2;
        }
        double *array = complexArray->data;
        double *out = fftw_malloc(count * sizeof(double));
        for (int i = 0; i < count; i = i + 2) {
            double temp = array[i];
            out[i / 2] = temp;
            out[count - i / 2] = temp;
        }
        return QCArrayCreateNoCopy(out, count, true);
    }
    return NULL;
}

QCArrayRef QCArraySquareSparsePoly(QCArrayRef array, int times) {
    if (array) {
        times = 1;

        int count = array->count;
        QCArrayRef indices = QCArrayGetNoZeroIndices(array);

        int mod = count;
        QCArrayRef result = QCArrayCreate(mod);
        double *x = result->data;
        int mul = (int) pow(2, times) % mod;
        QCArrayMultiply(indices, mul);

        int *d = indices->data;
        for (int i = 0; i < indices->count; ++i) {
            int index = d[i];
            int idx = index % mod;
            x[idx] = (int) x[idx] ^ 1;
        }

        QCArrayFree(indices);

        return result;
    }
    return NULL;
}

QCArrayRef QCArrayMulPoly(QCArrayRef x, QCArrayRef y) {
    int count = x->count;
    QCArrayRef fx = QCArrayFFT(x);
    QCArrayRef fy = QCArrayFFT(y);
    QCArrayRef mul = QCArrayComplexMultiply(fx, fy);
    QCArrayRef result = QCArrayInverseFFT(mul);
    QCArraySetCount(result, count);

    QCArrayRound(result);
    QCArrayMod(result, 2);

    QCArrayFree(fx);
    QCArrayFree(fy);
    QCArrayFree(mul);

    return result;
}

QCArrayRef QCArrayExpPoly(QCArrayRef array, int64_t n) {
    int length = array->count;
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

void QCArrayFree(QCArrayRef array) {
    if (array && array->needfree) {
        fftw_free(array->data);
    }
}
