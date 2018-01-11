//
// Created by Isaac on 2018/1/10.
//

#include "fft.h"
#include <stdlib.h>
#include <math.h>
#include <memory.h>

static void QCScaleArray(double *x, int size) {
    int i = 0;
    double bSi = 1.0 / size;
    while(i < size) {
        x[i] *= bSi; i++;
    }
}

double *QCFFT(double * x, int size) {
    double *out = fftw_malloc((size + 2) * sizeof(double));
    fftw_plan plan = fftw_plan_dft_r2c_1d(size, x, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}

double *QCInverseFFT(double * x, int size) {
    double *out = fftw_malloc((size - 2) * sizeof(double));
    QCScaleArray(x, size);
    fftw_plan plan = fftw_plan_dft_c2r_1d(size - 2, x, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}

double *QCRFFT(double * x, int size) {
    double *out = fftw_malloc(size * sizeof(double));
    fftw_plan plan = fftw_plan_r2r_1d(size, x, out, FFTW_R2HC, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}

double *QCInverseRFFT(double * x, int size) {
    double *out = fftw_malloc(size * sizeof(double));
    fftw_plan plan = fftw_plan_r2r_1d(size, x, out, FFTW_HC2R, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}


double *QCComplexMultiply(double * x, double * y, int size) {
    double *out = fftw_malloc(size * sizeof(double));
    for (int i = 0; i < size; i = i + 2) {
        double a = x[i];
        double b = x[i + 1];
        double c = y[i];
        double d = y[i + 1];
        out[i] = a * c - b * d;
        out[i + 1] = a * d + b * c;
    }
    if (size % 2 == 1) {
        out[size - 1] = x[size - 1] * y[size - 1];
    }
    return out;
}

double *QCArrayMultiply(double * x, double mul, int count) {
    for (int i = 0; i < count; ++i) {
        x[i] *= mul;
    }
    return x;
}

void QCArrayRound(double *x, int size) {
    for (int i = 0; i < size; ++i) {
        x[i] = round(x[i]);
    }
}

int *QCGetNoZeroIndices(double *x, int size, int *outsize) {
    int *indices = fftw_malloc(sizeof(int) * size);
    int idx = 0;
    for (int i = 0; i < size; ++i) {
        if ((int)round(x[i]) != 0) {
            indices[idx] = i;
            ++idx;
        }
    }

    int *result = NULL;
    if (idx > 0) {
        result = fftw_malloc(sizeof(int) * idx);
        memcpy(result, indices, sizeof(int) * idx);
    }
    *outsize = idx;

    fftw_free(indices);
    return result;
}

void QCArrayMod(double *x, int mod, int size) {
    for (int i = 0; i < size; ++i) {
        x[i] = (int)fabs(x[i]) % mod;
    }
}

double *QCGetRealParts(double *array, int size) {
    double *out = fftw_malloc(size * sizeof(double));
    for (int i = 0; i < size; i = i + 2) {
        double temp = array[i];
        out[i / 2] = temp;
        out[size - i / 2] = temp;
    }
    return out;
}

double *QCCreateZeroArray(int count) {
    size_t size = sizeof(double) * count;
    double *result = fftw_malloc(size);
    memset(result, 0, size);
    return result;
}