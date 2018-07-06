
#include "sort_file.h"


SR_ErrorCode swap_records(int,int,int);

SR_ErrorCode quickSort(int,int,int,int);

SR_ErrorCode id_partition(int,int,int);

SR_ErrorCode name_partition(int,int,int);

SR_ErrorCode sur_partition(int,int,int);

SR_ErrorCode get_id(int,int *,int);

SR_ErrorCode get_name(int,char *,int);

SR_ErrorCode get_sur(int,char *,int);

/*
void swap(Record *,Record *);

int id_partition(Record *,int,int);

int name_partition(Record *,int,int);

int surname_partition(Record *,int,int);

void id_quickSort(Record *,int,int);

void name_quickSort(Record *,int,int);

void surname_quickSort(Record *,int,int);
*/
