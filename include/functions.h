#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "sort_file.h"

SR_ErrorCode CopyFile(int fd1,int fd2);

void initArray(int (*A)[2],int row,int v);

SR_ErrorCode sortFilePart(int (*A)[2],int bufferSize,int fieldNo,int fd1,int fd2);

SR_ErrorCode sortFileQuickSort(int,int,int);


#endif
