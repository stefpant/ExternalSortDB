#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#include "bf.h"
#include "sort_file.h"
#include "QS.h"




SR_ErrorCode swap_records(int pos1,int pos2,int fd){
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	Record temp_rec;
	char *data1;
	char *data2;
	BF_Block *b1;
	BF_Block *b2;
	BF_Block_Init(&b1);
	BF_Block_Init(&b2);
	int block1 = pos1/max_records + 1;
	int block2 = pos2/max_records + 1;
	int p1 = pos1%max_records;
	int p2 = pos2%max_records;
	if(BF_GetBlock(fd,block1,b1) != BF_OK)
		return SR_ERROR;
	if(BF_GetBlock(fd,block2,b2) != BF_OK)
		return SR_ERROR;
	data1 = BF_Block_GetData(b1);
	data2 = BF_Block_GetData(b2);
	memcpy(&temp_rec,data1 + sizeof(int) + (p1 * record_size),record_size);
	memcpy(data1 + sizeof(int) + (p1 * record_size),data2 + sizeof(int) + (p2 * record_size),record_size);
	memcpy(data2 + sizeof(int) + (p2 * record_size),&temp_rec,record_size);
      	BF_Block_SetDirty(b1);
	BF_UnpinBlock(b1);
      	BF_Block_SetDirty(b2);
	BF_UnpinBlock(b2);
	BF_Block_Destroy(&b1);
	BF_Block_Destroy(&b2);
}



SR_ErrorCode get_id(int pos,int *nam,int fd){
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	Record temp_rec;
	BF_Block *block;
	BF_Block_Init(&block);
	int b = pos/max_records + 1;
	int p = pos%max_records;
	if(BF_GetBlock(fd,b,block) != BF_OK)
		return SR_ERROR;
	char *data;
	data = BF_Block_GetData(block);
//	memcpy(&nam,data + sizeof(int) + (p * record_size),sizeof(int));
	memcpy(&temp_rec,data + sizeof(int) + (p * record_size),record_size);
	memcpy(nam,&temp_rec.id,sizeof(int));
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
}

SR_ErrorCode get_name(int pos,char *nam,int fd){
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	Record temp_rec;
	BF_Block *block;
	BF_Block_Init(&block);
	int b = pos/max_records + 1;
	int p = pos%max_records;
	if(BF_GetBlock(fd,b,block) != BF_OK)
		return SR_ERROR;
	char *data;
	data = BF_Block_GetData(block);
	memcpy(&temp_rec,data + sizeof(int) + (p * record_size),record_size);
	memcpy(nam,&temp_rec.name,15*sizeof(char));
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
}

SR_ErrorCode get_sur(int pos,char *nam,int fd){
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	Record temp_rec;
	BF_Block *block;
	BF_Block_Init(&block);
	int b = pos/max_records + 1;
	int p = pos%max_records;
	if(BF_GetBlock(fd,b,block) != BF_OK)
		return SR_ERROR;
	char *data;
	data = BF_Block_GetData(block);
	memcpy(&temp_rec,data + sizeof(int) + (p * record_size),record_size);
	memcpy(nam,&temp_rec.surname,20*sizeof(char));
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
}

SR_ErrorCode quickSort(int low,int high,int fieldNo,int fd){
	if(low < high){
                if(fieldNo == 0){
			int pi = id_partition(low,high,fd);
			quickSort(low,pi - 1,fieldNo,fd);
			quickSort(pi + 1,high,fieldNo,fd);
		}
		else if(fieldNo == 1){
			int pi = name_partition(low,high,fd);
			quickSort(low,pi - 1,fieldNo,fd);
			quickSort(pi + 1,high,fieldNo,fd);
		}
		else if(fieldNo == 2){
			int pi = sur_partition(low,high,fd);
			quickSort(low,pi - 1,fieldNo,fd);
			quickSort(pi + 1,high,fieldNo,fd);
		}
	}
}

SR_ErrorCode id_partition(int low,int high,int fd){
	int pivot;
	get_id(high,&pivot,fd);
	//printf("%c%c%c%c \n",pivot[0],pivot[1],pivot[2],pivot[3]);
	int temp;
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	int i = low - 1;
	for(int j=low;j<=high-1;j++){
		get_id(j,&temp,fd);
		//printf("%c%c%c%c \n",temp[0],temp[1],temp[2],temp[3]);
		if(temp<=pivot){
			i++;
			swap_records(j,i,fd);
		}
	}
	swap_records(i+1,high,fd);
	return i+1;
}

SR_ErrorCode name_partition(int low,int high,int fd){
	char pivot[15];
	get_name(high,pivot,fd);
	//printf("%c%c%c%c \n",pivot[0],pivot[1],pivot[2],pivot[3]);
	char temp[15];
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	int i = low - 1;
	for(int j=low;j<=high-1;j++){
		get_name(j,temp,fd);
		//printf("%c%c%c%c \n",temp[0],temp[1],temp[2],temp[3]);
		if(strcmp(temp,pivot)<=0){
			i++;
			swap_records(j,i,fd);
		}
	}
	swap_records(i+1,high,fd);
	return i+1;
}

SR_ErrorCode sur_partition(int low,int high,int fd){
	char pivot[20];
	get_sur(high,pivot,fd);
	//printf("%c%c%c%c \n",pivot[0],pivot[1],pivot[2],pivot[3]);
	char temp[20];
	int record_size = sizeof(int) + (15+20+20)*sizeof(char);
	int max_records = (BF_BLOCK_SIZE - sizeof(int))/record_size;
	int i = low - 1;
	for(int j=low;j<=high-1;j++){
		get_sur(j,temp,fd);
		//printf("%c%c%c%c \n",temp[0],temp[1],temp[2],temp[3]);
		if(strcmp(temp,pivot)<=0){
			i++;
			swap_records(j,i,fd);
		}
	}
	swap_records(i+1,high,fd);
	return i+1;
}
































