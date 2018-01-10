//
// Created by Isaac on 2018/1/10.
//

#include "arithmetic.h"
#include "fft.h"

double *QCMulPoly(double * x, double * y, int count) {
    double *fx = QCFFT(x, count);
    double *fy = QCFFT(y, count);
    double *mul = QCComplexMultiply(x, y, count + 2);
    double *temp = QCInverseFFT(mul, count);
    double *real = QCGetRealParts(temp, count);
    QCArrayRound(real, count);
    QCArrayMod(real, 2, count);

    fftw_free(fx);
    fftw_free(fy);
    fftw_free(mul);
    fftw_free(temp);

    return real;
}

double *QCSquareSparsePoly(double *x, int time2) {

}

double *QCExpPoly(double *x, int n) {

}
