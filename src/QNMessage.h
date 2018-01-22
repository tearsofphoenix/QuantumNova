//
// Created by Isaac on 2018/1/16.
//

#ifndef QN_QNMESSAGE_H
#define QN_QNMESSAGE_H

typedef struct QNMessage *QNMessageRef;

extern QN_STRONG QNMessageRef QNMessageCreateFromPEMFile(const char *path);

#endif //QN_QNMESSAGE_H
