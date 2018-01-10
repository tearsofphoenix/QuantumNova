//
// Created by Isaac on 2018/1/10.
//

#ifndef PQC_CRYPTO_FFT_H
#define PQC_CRYPTO_FFT_H

#include <fftw3.h>
    extern double *QCFFT(double * x, int size);

    extern double *QCInverseFFT(double * x, int size);

    extern void QCArrayRound(double *x, int size);

    extern double *QCGetRealParts(double *complexArray, int size);

#endif //PQC_CRYPTO_FFT_H
