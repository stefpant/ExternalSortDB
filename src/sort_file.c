#include "sort_file.h"
#include "bf.h"
#include "functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SR_ID 123

SR_ErrorCode SR_Init() {

  return SR_OK;
}

SR_ErrorCode SR_CreateFile(const char *fileName) {
  int fd;
  BF_Block *block;
  BF_Block_Init(&block);
  char *data;
  int sr_id = SR_ID;//define as heap_id=86

  //if any critical function fails reterun error
  if(BF_CreateFile(fileName)!= BF_OK)
    return SR_ERROR;
  //we give identification for heap file
  if(BF_OpenFile(fileName,&fd)!= BF_OK)
    return SR_ERROR;
  if(BF_AllocateBlock(fd,block)!= BF_OK)
    return SR_ERROR;
  data = BF_Block_GetData(block);
  memcpy(data, (char*)&sr_id, sizeof(int));
  //maybe more matedata in this block later
  BF_Block_SetDirty(block);
  if(BF_UnpinBlock(block)!= BF_OK)
    BF_PrintError(BF_ERROR);
  if(BF_CloseFile(fd)!= BF_OK)
    return SR_ERROR;
  BF_Block_Destroy(&block);

  return SR_OK;
}

SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
  BF_Block *block;
  BF_Block_Init(&block);
  char *data;
  int counter;

  if(BF_OpenFile(fileName,fileDesc)!= BF_OK)
    return SR_ERROR;
  //now let's check if the file is sort file...
  if(BF_GetBlockCounter(*fileDesc,&counter) != BF_OK)
    return SR_ERROR;
  if(counter==0)//empty file
    return SR_ERROR;
  if(BF_GetBlock(*fileDesc,0,block) != BF_OK)
    return SR_ERROR;
  data = BF_Block_GetData(block);
  memcpy(&counter,data,sizeof(int));
  if(counter!=SR_ID)
    return SR_ERROR;
  if(BF_UnpinBlock(block) != BF_OK)
    BF_PrintError(BF_ERROR);
  BF_Block_Destroy(&block);

  return SR_OK;
}

SR_ErrorCode SR_CloseFile(int fileDesc) {
  if(BF_CloseFile(fileDesc)== BF_OK)
    return SR_OK;
  else
    return SR_ERROR;
}

SR_ErrorCode SR_InsertEntry(int fileDesc,	Record record) {
  int counter,rec_counter;
  int record_size = sizeof(int) + (15+20+20)*sizeof(char);
  int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
  BF_Block *block;
  BF_Block_Init(&block);
  char *data,*offset;
  if(BF_GetBlockCounter(fileDesc,&counter) != BF_OK)
    return SR_ERROR;
  if(counter==1){//first time allocate new block
    if(BF_AllocateBlock(fileDesc,block) != BF_OK)
      return SR_ERROR;
    rec_counter = 1;
    data = BF_Block_GetData(block);
  }
  else{//check if last block have space for the record else alloc new
    if(BF_GetBlock(fileDesc,counter-1,block) != BF_OK)
      return SR_ERROR;
    data = BF_Block_GetData(block);
    memcpy(&rec_counter,data,sizeof(int));
    if(rec_counter < max_records){//we'll insert record here
      rec_counter++;
    }
    else{//rec_counter == max_records
      if(BF_UnpinBlock(block) != BF_OK)//we just got+pinned a full
        BF_PrintError(BF_ERROR);// of records block so we unpin it
      if(BF_AllocateBlock(fileDesc,block) != BF_OK)
        return SR_ERROR;
      rec_counter = 1;
      data = BF_Block_GetData(block);
    }
  }
  //until here we collect info about the offset to write the record
  //offset == data + sizeof(int) + (rec_counter-1)*record_size
  //                  ^counter^      ^all previous records^
  memcpy(data, (char*)&rec_counter, sizeof(int));
  memcpy((data + (rec_counter-1)*record_size + sizeof(int)), &record.id, sizeof(int));
  memcpy((data + (rec_counter-1)*record_size + 2*sizeof(int)), record.name, 15*sizeof(char));
  memcpy((data + (rec_counter-1)*record_size + 2*sizeof(int) + 15*sizeof(char)), record.surname, 20*sizeof(char));
  memcpy((data + (rec_counter-1)*record_size + 2*sizeof(int) + 35*sizeof(char)), record.city, 20*sizeof(char));
  BF_Block_SetDirty(block);
  if(BF_UnpinBlock(block) != BF_OK)
    BF_PrintError(BF_ERROR);

  BF_Block_Destroy(&block);

  return SR_OK;
}

SR_ErrorCode SR_SortedFile(
  const char* input_filename,
  const char* output_filename,
  int fieldNo,
  int bufferSize
) {
  if(fieldNo<0 || fieldNo>3) return SR_ERROR;
  if(bufferSize>BF_BUFFER_SIZE) return SR_ERROR;

  int fd;
  SR_OpenFile(input_filename,&fd);

  int fd1;
  SR_CreateFile(output_filename);
  SR_OpenFile(output_filename,&fd1);
/*  int fd3;
  SR_CreateFile("File3");
  SR_OpenFile("File3",&fd3);*/
  CopyFile(fd,fd1);
  sortFileQuickSort(bufferSize,fieldNo,fd1);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//------------------QUICKSORT----------------------

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*  if(CopyFile(fd3,fd1) != SR_OK)
    return SR_ERROR;
  char rem2file[] = "rm -f File3";
  SR_CloseFile(fd3);
  if(system(rem2file) == -1)
    return SR_ERROR;*/

  int fd2;
  SR_CreateFile("temp_file.db");
  SR_OpenFile("temp_file.db",&fd2);
  char remfile[] = "rm -f temp_file.db";
  int flag=0;
  int total_counter,total_counter2;
  if(BF_GetBlockCounter(fd,&total_counter) != BF_OK)
    return SR_ERROR;
  total_counter--;//all data blocks without metadata
  total_counter2=total_counter;
  int block_counter=bufferSize;
  int Array[bufferSize-1][2];
  int v=0;
  while(1){
    initArray(Array,bufferSize-1,v);//init with v
    for(int i=0;i<bufferSize-1;i++){
      Array[i][0]+=i*block_counter;//put the previous block pointer
      if(total_counter2>block_counter){
        Array[i][1]=block_counter;
        total_counter2-=block_counter;
      }
      else{
        Array[i][1]=total_counter2;
        total_counter2=total_counter;
        flag=1;
        break;
      }
    }
    v+=(bufferSize-1)*block_counter;
    if(sortFilePart(Array,bufferSize,fieldNo,fd1,fd2) != SR_OK)
      return SR_ERROR;
    if(flag){
      v=0;
      block_counter*=bufferSize-1;
      flag=0;
      CopyFile(fd2,fd1);
      SR_CloseFile(fd2);
      if(system(remfile) == -1)
        return SR_ERROR;
      if(block_counter>total_counter) break;//final break file is sorted now
      SR_CreateFile("temp_file.db");
      SR_OpenFile("temp_file.db",&fd2);
    }
  }
  SR_CloseFile(fd);
  SR_CloseFile(fd1);
  return SR_OK;
}

SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
  int counter,i,j;
  int record_size = sizeof(int) + (15+20+20)*sizeof(char);
  int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
  BF_Block *block;
  BF_Block_Init(&block);
  Record record;
  char *data;
  int last_rec_counter;

  if(BF_GetBlockCounter(fileDesc,&counter) != BF_OK)
    return SR_ERROR;
  for(i=1; i<counter-1; i++){//for all full blocks
    if(BF_GetBlock(fileDesc,i,block) != BF_OK)
      return SR_ERROR;
    data = BF_Block_GetData(block);
    for(j=0; j<max_records; j++){
      memcpy(&(record.id), data + j*record_size + sizeof(int), sizeof(int));
      memcpy(record.name, data + j*record_size + 2*sizeof(int), 15*sizeof(char));
      memcpy(record.surname, data + j*record_size + 2*sizeof(int) + 15*sizeof(char), 20*sizeof(char));
      memcpy(record.city, data + j*record_size + 2*sizeof(int) + 35*sizeof(char), 20*sizeof(char));

      printf("record:%d-->",(i-1)*max_records+j+1);
      printf("%d,\"%s\",\"%s\",\"%s\"\n",record.id,record.name,record.surname,record.city);
    }
    if(BF_UnpinBlock(block) != BF_OK)
      BF_PrintError(BF_ERROR);
  }
  if(BF_GetBlock(fileDesc,counter-1,block) != BF_OK)
    return SR_ERROR;
  data = BF_Block_GetData(block);
  memcpy(&last_rec_counter, data,  sizeof(int));//number of records in this(last) block
  for(j=0; j<last_rec_counter; j++){//for the last block
    memcpy(&record.id, data + j*record_size + sizeof(int), sizeof(int));
    memcpy(record.name, data + j*record_size + 2*sizeof(int), 15*sizeof(char));
    memcpy(record.surname, data + j*record_size + 2*sizeof(int) + 15*sizeof(char), 20*sizeof(char));
    memcpy(record.city, data + j*record_size + 2*sizeof(int) + 35*sizeof(char), 20*sizeof(char));

    printf("record:%d-->",(counter-2)*max_records+j+1);
    printf("%d,\"%s\",\"%s\",\"%s\"\n",record.id,record.name,record.surname,record.city);
  }
  if(BF_UnpinBlock(block) != BF_OK)
    BF_PrintError(BF_ERROR);

  BF_Block_Destroy(&block);

  return SR_OK;
}
