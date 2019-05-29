#include "file_sys.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>
int now_dir_id = ROOT_ID;

static void clear_bitgraph()
{
	char info[SECTOR_SIZE] = { 0 };
	for (int i = 0; i < BITGRAPH_SECTOR_NUM; ++i) {
		save_data(BITGRAPH_START_SECTOR + i, 1, info);
	}
}

//Return the id is relative to file sectors.
static int get_free_sector()
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
static void use_sector(int file_section_id)
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

static void clear_file_sector(int id)
{
	char info[SECTOR_SIZE] = { 0 };
	save_data(FILE_SECTOR(id), 1, info);
}

static int add_file_sector(int type, int father_id, int last_id, char *name,FILE_HEADER* file_header)
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

static void add_item(int father_id, int son_id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(father_id), 1, info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	while (header->used_size + ID_SIZE > SECTOR_SIZE) {
		if (header->next_id!=-1) {
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

static void create_file(int type,int father_id,char* name)
{
	
	FILE_HEADER file_header;
	int id=add_file_sector(type, father_id, -1, name, &file_header);
	if (father_id != -1) {
		add_item(father_id, id);
		
	}
}

static int get_item_num(FILE_HEADER* header)
{
	return (header->used_size - header->name_len - 1 - sizeof(FILE_HEADER)) / ID_SIZE;
}

static void print_file_name(int id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(id), 1, info);
	char* name = info + sizeof(FILE_HEADER);
	FILE_HEADER* header = (FILE_HEADER*)info;
	if (header->type == DIR_TYPE) {
		printf("%/");
	}
	printf("%s ", name);
}

int ls(int argc, char* argv[])
{
	char info[SECTOR_SIZE];
	int id = now_dir_id;
	while (id!=-1) {
		get_data(FILE_SECTOR(id), 1, info);
		FILE_HEADER* header = (FILE_HEADER*)info;
		int* item_id = (int*)(info + sizeof(FILE_HEADER) + header->name_len + 1);
		int num = get_item_num(header);
		for (int i = 0; i < num; ++i) {
			print_file_name(item_id[i]);
		}
		id = header->next_id;
	}
	printf("\n");
	return 0;
}

int init_file_sys(int argc, char* argv[])
{
	clear_file_sector(FILE_SECTOR(ROOT_ID));
	clear_bitgraph();
	create_file(DIR_TYPE, -1, "");
	return 0;
}

void test()
{
	init_file_sys(1,NULL);
	for (int i = 0; i < 25; ++i) {
		char k[2] = "a";
		k[0] = 'a' + i;
		create_file(DIR_TYPE, 0, k);
	}
	create_file(DIR_TYPE, 0, "Zhou");
	ls(0, NULL);
}