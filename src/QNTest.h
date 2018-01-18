//
// Created by Isaac on 2018/1/18.
//

#ifndef PQC_CRYPTO_QNTEST_H
#define PQC_CRYPTO_QNTEST_H

#include <stddef.h>
#include <stdbool.h>

typedef bool (* QNTestFunc)(void);

struct QNTest {
    const char *name;
    const char *description;
    const QNTestFunc func;
    const size_t loopCount;
};

typedef struct QNTest *QNTestRef;

extern void QNTestRun(QNTestRef test);

#ifndef QNT
#define QNT(N, D, F, C) do { \
                                struct QNTest t = {N, D, F, C}; \
                                QNTestRun(&t); \
                           } while(0)
#endif

#endif //PQC_CRYPTO_QNTEST_H
