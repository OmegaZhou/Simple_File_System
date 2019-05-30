#include "file_sys_config.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>
int now_dir_id = ROOT_ID;

void clear_bitgraph()
{
	char info[SECTOR_SIZE] = { 0 };
	for (int i = 0; i < BITGRAPH_SECTOR_NUM; ++i) {
		save_data(BITGRAPH_START_SECTOR + i, 1, info);
	}
}

//Return the id is relative to file sectors.
int get_free_sector()
{
	char info[SECTOR_SIZE] = { 0 };
	for (int i = 0; i < BITGRAPH_SECTOR_NUM; ++i) {
		get_data(BITGRAPH_START_SECTOR + i, 1, info);
		for (int j = 0; j < SECTOR_SIZE; ++j) {
			for (int k = 0; k < 8; ++k) {
				if ((((info[j]) ^ (1 << k))&(1 << k)) == (1 << k)) {
					return i * SECTOR_SIZE * 8 + j * 8 + k;
				}
			}
		}
	}
	return -1;
}

//file_section_id is relative to file sectors
void use_sector(int file_section_id)
{
	int k = file_section_id % 8;
	file_section_id /= 8;
	int j = file_section_id % SECTOR_SIZE;
	int i = file_section_id / SECTOR_SIZE + BITGRAPH_START_SECTOR;
	char info[SECTOR_SIZE] = { 0 };
	get_data(i, 1, info);
	info[j] |= (1 << k);
	save_data(i, 1, info);
}

void clear_file_sector(int id)
{
	char info[SECTOR_SIZE] = { 0 };
	save_data(FILE_SECTOR(id), 1, info);
}

int add_file_sector(int type, int father_id, int last_id, char *name, FILE_HEADER* file_header)
{
	int len = strlen(name);
	int id = get_free_sector();
	use_sector(id);
	file_header->type = type;
	file_header->last_id = last_id;
	file_header->next_id = -1;
	file_header->name_len = len;
	file_header->father_id = father_id;
	file_header->now_id = id;
	file_header->used_size = sizeof(FILE_HEADER) + len + 1;
	char data[SECTOR_SIZE] = { 0 };
	char* buf = data;
	char *temp = (char*)file_header;
	for (int i = 0; i < sizeof(FILE_HEADER); ++i) {
		*buf = *temp;
		++buf;
		++temp;
	}
	for (int i = 0; i < len; ++i) {
		*buf = *name;
		++buf;
		++name;
	}
	*buf = '\0';
	save_data(FILE_SECTOR(id), 1, data);
	if (file_header->last_id != -1) {
		get_data(FILE_SECTOR(file_header->last_id), 1, data);
		FILE_HEADER* header = (FILE_HEADER*)data;
		header->next_id = id;
		save_data(FILE_SECTOR(file_header->last_id), 1, data);
	}
	return id;
}

void add_item(int father_id, int son_id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(father_id), 1, info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	while (header->used_size + ID_SIZE > SECTOR_SIZE) {
		if (header->next_id != -1) {
			get_data(FILE_SECTOR(header->next_id), 1, info);
		} else {
			int id = add_file_sector(header->type, header->father_id, header->now_id, header + 1, header);
			get_data(FILE_SECTOR(id), 1, info);
		}
		header = (FILE_HEADER*)info;
	}

	int* data = (int*)(info + header->used_size);
	*data = son_id;
	header->used_size += ID_SIZE;
	save_data(FILE_SECTOR(header->now_id), 1, info);
}

void get_file_name(int father_id, char* name)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(father_id), 1, info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	char* temp = NAME_START_LOC(info);
	while (*temp != '\0') {
		*name = *temp;
		++temp;
		++name;
	}
	*name = '\0';
}

int get_item_num(FILE_HEADER* header)
{
	return (header->used_size - header->name_len - 1 - sizeof(FILE_HEADER)) / ID_SIZE;
}

int check_file_name(int father_id, char* name)
{
	char item_name[SECTOR_SIZE];
	char info[SECTOR_SIZE];
	int id = father_id;
	while (id != -1) {
		get_data(FILE_SECTOR(id), 1, info);
		FILE_HEADER* header = (FILE_HEADER*)info;
		char* temp = NAME_START_LOC(info);
		int num = get_item_num(header);
		int* data = (int*)DATA_START_LOC(info, header->name_len);
		for (int i = 0; i < num; ++i) {
			get_file_name(data[i], item_name);
			if (strcmp(item_name, name) == 0) {
				return 0;
			}
		}
		id = header->next_id;
	}
	return 1;
}


int create_file(int type, int father_id, char* name)
{
	if (father_id != -1 && (!check_file_name(father_id, name))) {
		return 0;
	}
	FILE_HEADER file_header;
	int id = add_file_sector(type, father_id, -1, name, &file_header);
	if (father_id != -1) {
		add_item(father_id, id);

	}
	return 1;
}



void print_file_name(int id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(id), 1, info);
	char* name = NAME_START_LOC(info);;
	FILE_HEADER* header = (FILE_HEADER*)info;
	if (header->type == DIR_TYPE) {
		printf("%/");
	}
	printf("%s", name);
}
