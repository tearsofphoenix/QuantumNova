//
// Created by Isaac on 2018/1/18.
//

#include "QNTest.h"
#include <time.h>
#include <stdio.h>

#define kLine "----------"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void QNTestRun(QNTestRef test) {
    if (test) {
        printf(KNRM kLine "%s test start" kLine "\n", test->name);
        const QNTestFunc func = test->func;
        if (test->loopCount > 0 && func) {
            printf(KBLU "| will loop %d times \n", test->loopCount);
            bool success = true;
            clock_t start = clock();
            for (size_t i = 0; i < test->loopCount; ++i) {
                if(func()) {
                    // success one time
                } else {
                    printf(KRED "| fail at %d-th loop.\n", i);
                    success = false;
                    break;
                }
            }
            clock_t end = clock();
            if (success) {
                float total = (float)(end - start) / 1000;
                printf(KBLU "| test passed. cost: %.02fms average: %.02fms.\n", total, total / test->loopCount);
            }
        }
        printf(KNRM kLine "%s" kLine "\n\n\n", "test end");
    }
}
