//
// Created by Isaac on 2018/1/11.
//

#include "QCArray.h"
#include <fftw3.h>
#include <memory.h>
#include <math.h>

struct QCArray {
    double *data;
    int count;
};

QCArrayRef QCArrayCreate(int count) {
    QCArrayRef array = fftw_malloc(sizeof(*array));
    array->data = fftw_alloc_real(count);
    memset(array->data, 0, sizeof(double) * count);
    array->count = count;
    return array;
}

QCArrayRef QCArrayCreateFrom(const double *x, int count) {
    QCArrayRef array = fftw_malloc(sizeof(*array));
    array->data = fftw_alloc_real(count);
    memcpy(array->data, x, sizeof(double) * count);
    array->count = count;
    return array;
}

QCArrayRef QCArrayCreateNoCopy(double *x, int count) {
    QCArrayRef array = fftw_malloc(sizeof(*array));
    array->data = x;
    array->count = count;
    return array;
}

QCArrayRef QCArrayFFT(QCArrayRef array) {
    int count = array->count;
    double *out = fftw_malloc((count + 2) * sizeof(double));
    fftw_plan plan = fftw_plan_dft_r2c_1d(count, array->data, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return QCArrayCreateNoCopy(out, count + 2);
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
        double *out = fftw_malloc((count - 2) * sizeof(double));

        QCArrayScale(array, 1.0 / count);
        fftw_plan plan = fftw_plan_dft_c2r_1d(count - 2, array->data, out, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        return QCArrayCreateNoCopy(out, count - 2);
    }
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
        return QCArrayCreateNoCopy(out, count);
    }
    return  NULL;
}


void QCArrayMultiply(QCArrayRef array, double mul) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        for (int i = 0; i < count; ++i) {
            x[i] *= mul;
        }
    }
}

void QCArrayRound(QCArrayRef array) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        for (int i = 0; i < count; ++i) {
            x[i] = round(x[i]);
        }
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
        double *array = complexArray->data;
        double *out = fftw_malloc(count * sizeof(double));
        for (int i = 0; i < count; i = i + 2) {
            double temp = array[i];
            out[i / 2] = temp;
            out[count - i / 2] = temp;
        }
        return QCArrayCreateNoCopy(out, count);
    }
    return NULL;
}

QCArrayRef QCArrayGetNoZeroIndices(QCArrayRef array) {
    if (array) {
        int count = array->count;
        double *x = array->data;
        int *indices = fftw_malloc(sizeof(int) * count);
        int idx = 0;
        for (int i = 0; i < count; ++i) {
            if ((int) round(x[i]) != 0) {
                indices[idx] = i;
                ++idx;
            }
        }

        int *result = NULL;
        if (idx > 0) {
            result = fftw_malloc(sizeof(int) * idx);
            memcpy(result, indices, sizeof(int) * idx);
        }

        fftw_free(indices);

        return QCArrayCreateNoCopy(result, idx);
    }
    return NULL;
}

QCArrayRef QCArraySquareSparsePoly(QCArrayRef array, int times) {
    times = 1;

    int count = array->count;
    QCArrayRef indices = QCArrayGetNoZeroIndices(array);

    int mod = count;
    QCArrayRef result = QCArrayCreate(mod);
    double *x = result->data;
    int mul = (int)pow(2, times) % mod;
    QCArrayMultiply(indices, mul);

    for (int i = 0; i < indices->count; ++i) {
        int index = indices->data[i];
        int idx = index % mod;
        x[idx] = (int)x[idx] ^ 1;
    }

    QCArrayFree(indices);

    return result;
}

static bool _arrayCompare(const double * array, const double *expected, int count) {
    bool equal = true;
    int total = 0;
    for (int i = 0; i < count; ++i) {
        if (fabs(array[i] - expected[i]) > 0.00000005) {
            printf("not equal: %d %f %f\n", i, array[i], expected[i]);
            equal = false;
            ++total;
        }
    }
    if (!equal) {
        printf("total not equal: %d rate: %.2f%%", total, total * 100.0 / count);
    }
    return equal;
}

bool QCArrayCompare(QCArrayRef xArray, QCArrayRef yArray) {
    int count = xArray->count;
    double *array = xArray->data;
    double *expected = yArray->data;
    return _arrayCompare(array, expected, count);
}

bool QCArrayCompareRaw(QCArrayRef x, const double *expected) {
    int count = x->count;
    double *array = x->data;
    return _arrayCompare(array, expected, count);
}

void QCArrayPrint(QCArrayRef array) {
    int count = array->count;
    double *data = array->data;
    int padding = 25;
    printf("\n[ ");
    for (int i = 0; i < count; ++i) {
        printf("%f, ", data[i]);
        if (i % padding == 0 && i > 0) {
            printf("\n");
        }
    }
    printf(" ]\n");
}

void QCArrayFree(QCArrayRef array) {
    if (array) {
        fftw_free(array->data);
    }
}
