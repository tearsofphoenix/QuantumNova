//
// Created by Isaac on 2018/1/10.
//

#include <math.h>
#include <stdlib.h>
#include "arithmetic.h"
#include "fft.h"

double *QCMulPoly(double * x, double * y, int count) {
    double *fx = QCFFT(x, count);
    double *fy = QCFFT(y, count);
    double *mul = QCComplexMultiply(fx, fy, count + 2);
    double *temp = QCInverseFFT(mul, count + 2);
    double *real = QCGetRealParts(temp, count);
//    QCArrayRound(real, count);
//    QCArrayMod(real, 2, count);

    fftw_free(fx);
    fftw_free(fy);
    fftw_free(mul);
    fftw_free(temp);

    return real;
}

double *QCSquareSparsePoly(double *x, int count, int times) {
    times = 1;
    int indicesSize = 0;
    int *indices = QCGetNoZeroIndices(x, count, &indicesSize);

    int mod = count;
    double *result = QCCreateZeroArray(mod);
    int mul = (int)pow(2, times) % mod;
    QCArrayMultiply(indices, mul, count);

    for (int i = 0; i < indicesSize; ++i) {
        int index = indices[i];
        int idx = index % mod;
        result[idx] = (int)result[idx] ^ 1;
    }

    fftw_free(indices);

    return result;
}

double *QCExpPoly(double *x, int n) {

}
