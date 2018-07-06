#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#include "bf.h"
#include "sort_file.h"
#include "QS.h"


SR_ErrorCode CopyFile(int fd1,int fd2){
  BF_Block *block;
  BF_Block_Init(&block);
  int c1,c2,flag=0;
  if(BF_GetBlockCounter(fd1,&c1) != BF_OK)
    return SR_ERROR;
  if(BF_GetBlockCounter(fd2,&c2) != BF_OK)
    return SR_ERROR;
  if(c1==c2) flag=1;
  //char *temp;
  char *data1;
  char *data2;
  //temp = malloc(BF_BLOCK_SIZE*sizeof(char));
  for(int i=1;i<c1;i++){//copy all data block from fd1 to fd2
    if(BF_GetBlock(fd1,i,block) != BF_OK)
      return SR_ERROR;
    data1 = BF_Block_GetData(block);
    //memcpy(temp,data1,BF_BLOCK_SIZE*sizeof(char))
    BF_UnpinBlock(block);
    if(flag){
      if(BF_GetBlock(fd2,i,block) != BF_OK)
        return SR_ERROR;
    }
    else{
      if(BF_AllocateBlock(fd2,block)!= BF_OK)
        return SR_ERROR;
    }
    data2 = BF_Block_GetData(block);
    memcpy(data2,data1,BF_BLOCK_SIZE*sizeof(char));
    //memcpy(data2,temp,BF_BLOCK_SIZE*sizeof(char));
    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
  }
  //free(temp);
  BF_Block_Destroy(&block);
  return SR_OK;
}

void initArray(int (*A)[2],int row,int v){
  for(int i=0;i<row;i++){
      A[i][0]=v;
      A[i][1]=0;
  }
}

SR_ErrorCode sortFilePart(int (*A)[2],int bufferSize,int fieldNo,int fd1,int fd2){
  int record_size = sizeof(int) + (15+20+20)*sizeof(char);
  int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
  BF_Block *block;
  BF_Block_Init(&block);
  char *data;
  Record rec;
  int curf1,ftocheck1;
  char curf2[20],ftocheck2[20];
  int pos,flag=0;
  int countA[bufferSize-1][2];
  for(int i=0;i<bufferSize-1;i++){
    countA[i][0]=0;
    countA[i][1]=0;
  }
  while(1){
    pos=0;
    while(countA[pos][0]==0 && A[pos][1]==0 && pos<bufferSize-1) pos++;
    if(pos==bufferSize-1) break;//no more records we are done
    if(countA[pos][0]==0){//then A[pos][1]>1
      A[pos][1]--;
      A[pos][0]++;
      if(BF_GetBlock(fd1,A[pos][0],block) != BF_OK)
        return SR_ERROR;
      data = BF_Block_GetData(block);
      memcpy(&(countA[pos][0]),data,sizeof(int));
      countA[pos][1]=countA[pos][0];
    }
    else{
      if(BF_GetBlock(fd1,A[pos][0],block) != BF_OK)
        return SR_ERROR;
      data = BF_Block_GetData(block);
    }
    memcpy(&rec.id,data + sizeof(int) + (countA[pos][1]-countA[pos][0])*record_size,sizeof(int));
    memcpy(rec.name,data + 2*sizeof(int) + (countA[pos][1]-countA[pos][0])*record_size,15*sizeof(char));
    memcpy(rec.surname,data + 2*sizeof(int) + 15*sizeof(char) + (countA[pos][1]-countA[pos][0])*record_size,20*sizeof(char));
    memcpy(rec.city,data + 2*sizeof(int) + 35*sizeof(char) + (countA[pos][1]-countA[pos][0])*record_size,20*sizeof(char));
    if(fieldNo==0)
      curf1=rec.id;
    else if(fieldNo==1)
      strcpy(curf2,rec.name);
    else if(fieldNo==2)
      strcpy(curf2,rec.surname);
    else if(fieldNo==3)
      strcpy(curf2,rec.city);
    BF_UnpinBlock(block);
    for(int i=pos+1;i<bufferSize-1;i++){
      if(countA[i][0]==0){
        if(A[i][1]==0) continue;
        A[i][1]--;
        A[i][0]++;
        if(BF_GetBlock(fd1,A[i][0],block) != BF_OK)
          return SR_ERROR;
        data = BF_Block_GetData(block);
        memcpy(&(countA[i][0]),data,sizeof(int));
        countA[i][1]=countA[i][0];
      }
      else{
        if(BF_GetBlock(fd1,A[i][0],block) != BF_OK)
          return SR_ERROR;
        data = BF_Block_GetData(block);
      }
      if(fieldNo==0){
        memcpy(&ftocheck1,data + sizeof(int) + (countA[pos][1]-countA[i][0])*record_size,sizeof(int));
        if(ftocheck1<curf1){
          curf1=ftocheck1;
          flag=1;
        }
      }
      else{
        if(fieldNo==1)
          memcpy(ftocheck2,data + 2*sizeof(int) + (countA[i][1]-countA[i][0])*record_size,15*sizeof(char));
        else if(fieldNo==2)
          memcpy(ftocheck2,data + 2*sizeof(int) + 15*sizeof(char) + (countA[i][1]-countA[i][0])*record_size,20*sizeof(char));
        else if(fieldNo==3)
          memcpy(ftocheck2,data + 2*sizeof(int) + 35*sizeof(char) + (countA[i][1]-countA[i][0])*record_size,20*sizeof(char));
        if(strcmp(ftocheck2,curf2)<0){
          flag=1;
          strcpy(curf2,ftocheck2);
        }
      }
      if(flag){
        memcpy(&rec.id,data + sizeof(int) + (countA[i][1]-countA[i][0])*record_size,sizeof(int));
        memcpy(rec.name,data + 2*sizeof(int) + (countA[i][1]-countA[i][0])*record_size,15*sizeof(char));
        memcpy(rec.surname,data + 2*sizeof(int) + 15*sizeof(char) + (countA[i][1]-countA[i][0])*record_size,20*sizeof(char));
        memcpy(rec.city,data + 2*sizeof(int) + 35*sizeof(char) + (countA[i][1]-countA[i][0])*record_size,20*sizeof(char));
        flag=0;
        pos=i;
      }
      BF_UnpinBlock(block);
    }
    countA[pos][0]--;
    SR_InsertEntry(fd2,rec);
  }
  BF_Block_Destroy(&block);
  return SR_OK;
}



SR_ErrorCode sortFileQuickSort(int bufferSize,int fieldNo,int fd){
  int total_blocks;//Total blocks in the file
  if(BF_GetBlockCounter(fd,&total_blocks) != BF_OK)
    return SR_ERROR;
  int record_size = sizeof(int) + (15+20+20)*sizeof(char);
  int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
  Record r2p;
  BF_Block *block;
//  printf("%d \n",total_blocks);
  BF_Block_Init(&block);
  char *data;
  int sets = total_blocks / bufferSize;//(BF_BUFFER_SIZE - 1);
  int last_set = (total_blocks-1) % bufferSize;//(BF_BUFFER_SIZE - 1);

  if(last_set==0){sets--;last_set=bufferSize;}

  int records_in_set = bufferSize * max_records;
  for(int i =0;i<=sets - 1;i++){
    quickSort((i*records_in_set),((i+1) * records_in_set) - 1,fieldNo,fd);
  }
  int last_records = (last_set-1) * max_records;//last set wont be 0
  if(BF_GetBlock(fd,total_blocks-1,block) != BF_OK)
    return SR_ERROR;
  data = BF_Block_GetData(block);
  int c;
  memcpy(&c,data,sizeof(int));//check how many records we have in our last block
  last_records += c;
  quickSort((sets*records_in_set),(sets*records_in_set)+last_records-1,fieldNo,fd);//quicksort our last blocks
  return SR_OK;
}
