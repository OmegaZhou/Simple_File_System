#ifndef FILE_SYS_CONFIG_H_
#define FILE_SYS_CONFIG_H_
#include "disk.h"
#define DIR_TYPE 1
#define FILE_TYPE 0

#define MAX_FILE_NUM (SECTOR_SIZE * (MAX_SECTOR_ID - 2) / (1 + SECTOR_SIZE))
#define BITGRAPH_START_SECTOR 1
#define BITGRAPH_SECTOR_NUM (MAX_FILE_NUM/8/SECTOR_SIZE)
#define FILE_START_SECTOR (BITGRAPH_START_SECTOR+BITGRAPH_SECTOR_NUM)
#define FILE_SECTOR_NUM (BITGRAPH_SECTOR_NUM*8*SECTOR_SIZE)

#define ROOT_ID 0
#define FILE_SECTOR(id) ((id)+FILE_START_SECTOR)

#define ID_SIZE (sizeof(int))


typedef struct File_Header
{
	int type;
	int father_id;
	int now_id;
	int last_id;
	int next_id;
	int name_len;
	int used_size;
}FILE_HEADER;

#define NAME_START_LOC(info) ((info)+sizeof(FILE_HEADER))
#define DATA_START_LOC(info,len) (NAME_START_LOC(info)+(len)+1)

#endif // !FILE_SYS_CONFIG_H_


