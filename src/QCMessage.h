//
// Created by Isaac on 2018/1/16.
//

#ifndef PQC_CRYPTO_QCMESSAGE_H
#define PQC_CRYPTO_QCMESSAGE_H

typedef struct QCMessage *QCMessageRef;

extern QN_STRONG QCMessageRef QCMessageCreateFromPEMFile(const char *path);

#endif //PQC_CRYPTO_QCMESSAGE_H
