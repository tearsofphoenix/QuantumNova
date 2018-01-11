//
// Created by Isaac on 2018/1/11.
//

#include "QCArray.h"
#include "QCArrayPrivate.h"

#include <fftw3.h>
#include <memory.h>
#include <math.h>

static void *_QCMallocData(QCArrayDataType type, int count) {
    size_t size = 0;
    switch (type) {
        case QCDTInt: {
            size = sizeof(int) * count;
            break;
        }
        case QCDTFloat: {
            size = sizeof(float) * count;
            break;
        }
        case QCDTDouble: {
            size = sizeof(double) * count;
            break;
        }
    }
    void *data = fftw_malloc(size);
    memset(data, 0, size);
    return data;
}

QCArrayRef QCArrayCreate(int count) {
    return QCArrayCreateWithType(QCDTDouble, count);
}

QCArrayRef QCArrayCreateWithType(QCArrayDataType type, int count) {
    QCArrayRef array = fftw_malloc(sizeof(*array));
    array->data = _QCMallocData(type, count);
    array->count = count;
    array->fft = false;
    array->needfree = true;
    array->datatype = type;
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

void QCArraySetCount(QCArrayRef array, int newCount) {
    if (array) {
        array->count = newCount;
    }
}

void QCArraySetValueAt(QCArrayRef array, int index, double value) {
    if (array && index < array->count) {
        double *x = array->data;
        x[index] = value;
    }
}

double QCArrayValueAt(QCArrayRef array, int index) {
    if (array) {
        switch (array->datatype) {
            case QCDTInt: {
                int *d = array->data;
                return d[index];
            }
            default: {
                double *d = array->data;
                return d[index];
            }
        }
    }
    return 0;
}

int QCArrayGetNonZeroCount(QCArrayRef array) {
    if (array) {
        int total = 0;
        int count = 0;
        switch (array->datatype) {
            case QCDTInt: {
                int *d = array->data;
                for (int i = 0; i < count; ++i) {
                    if (d[i] != 0) {
                        ++total;
                    }
                }
                break;
            }
            default: {
                double *d = array->data;
                for (int i = 0; i < count; ++i) {
                    if ((int)d[i] != 0) {
                        ++total;
                    }
                }
                break;
            }
        }
        return total;
    }
    return 0;
}

void QCArraySetFFTFlag(QCArrayRef array, bool flag) {
    if (array) {
        array->fft = flag;
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


void QCArrayMultiply(QCArrayRef array, double mul) {
    if (array) {
        int count = array->count;
        switch (array->datatype) {
            case QCDTInt: {
                int *x = array->data;
                for (int i = 0; i < count; ++i) {
                    x[i] *= mul;
                }
                break;
            }
            default: {
                double *x = array->data;
                for (int i = 0; i < count; ++i) {
                    x[i] *= mul;
                }
                break;
            }
        }
    }
}

void QCArrayRound(QCArrayRef array) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        for (int i = 0; i < count; ++i) {
            x[i] = (int)round(x[i]);
        }
        array->datatype = QCDTInt;
    }
}

void QCArrayMod(QCArrayRef array, int mod) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        for (int i = 0; i < count; ++i) {
            x[i] = (int) x[i] % mod;
        }
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

QCArrayRef QCArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        int *indices = _QCMallocData(QCDTInt, count);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if ((int) round(x[i]) != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        int *result = NULL;
        if (idx > 0) {
            result = _QCMallocData(QCDTInt, idx);
            memcpy(result, indices, sizeof(int) * idx);
        }

        fftw_free(indices);

        QCArrayRef ref = QCArrayCreateNoCopy((void *)result, idx, true);
        ref->datatype = QCDTInt;
        return ref;
    }
    return NULL;
}

int QCArrayFindIndex(QCArrayRef array, int value) {
    if (array) {
        int count = array->count;
        int *data = array->data;
        for (int i = 0; i < count; ++i) {
            if (data[i] == value) {
                return i;
            }
        }
    }
    return -1;
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
