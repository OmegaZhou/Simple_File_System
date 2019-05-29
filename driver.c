#include <stdio.h>
#include "disk.h"
void get_data(int sector_id, int sector_num, char* info)
{
	FILE* file = fopen("file", "r");
	fseek(file, sector_id*SECTOR_SIZE, SEEK_SET);
	fread(info, sector_num*SECTOR_SIZE, 1, file);
	fclose(file);
}
void save_data(int sector_id, int sector_num, char* info)
{
	char k[SECTOR_SIZE];
	FILE* file = fopen("file", "rb+");
	fseek(file, sector_id*SECTOR_SIZE, SEEK_SET);
	for (int i = 0; i < SECTOR_SIZE; ++i) {
		fputc(info[i], file);
	}
	fclose(file);
}