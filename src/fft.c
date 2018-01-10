//
// Created by Isaac on 2018/1/10.
//

#include "fft.h"
#include <fftw3.h>
#include <stdlib.h>
#include <math.h>

double *QCFFT(double * x, int size) {
    double *out = fftw_malloc((size + 2) * sizeof(double));
    fftw_plan plan = fftw_plan_dft_r2c_1d(size, x, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}

double *QCInverseFFT(double * x, int size) {
    double *out = fftw_malloc(size * sizeof(double));
    fftw_plan plan = fftw_plan_dft_c2r_1d(size, x, out, FFTW_ESTIMATE);
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

void QCArrayRound(double *x, int size) {
    for (int i = 0; i < size; ++i) {
        x[i] = round(x[i]);
    }
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