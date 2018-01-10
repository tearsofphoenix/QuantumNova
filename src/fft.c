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
    double *out = fftw_malloc(size);
    fftw_plan plan = fftw_plan_dft_c2r_1d(size, x, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}

void QCArrayRound(double *x, int size) {
    for (int i = 0; i < size; ++i) {
        x[i] = round(x[i]);
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