//
// Created by Isaac on 2018/1/10.
//

#ifndef PQC_CRYPTO_FFT_H
#define PQC_CRYPTO_FFT_H

#include <fftw3.h>
    extern double *QCFFT(double * x, int count);

    extern double *QCInverseFFT(double * x, int count);

    extern double *QCRFFT(double * x, int count);

    extern double *QCInverseRFFT(double * x, int count);

    extern double *QCComplexMultiply(double * x, double * y, int count);

    extern double *QCArrayMultiply(double * x, double mul, int count);

    extern void QCArrayRound(double *x, int count);

    extern void QCArrayMod(double *x, int mod, int count);

    extern double *QCGetRealParts(double *complexArray, int count);

    extern int *QCGetNoZeroIndices(double *x, int count, int *outsize);

    extern double *QCCreateZeroArray(int count);

#endif //PQC_CRYPTO_FFT_H
