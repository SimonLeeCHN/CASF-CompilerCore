#ifndef COMPILERCORE_H
#define COMPILERCORE_H
#include <QList>

int InstructTranslate(QList<QString> src,QList<QString> &out,QList<int> &carrierPos,int rfidNum);

#endif
