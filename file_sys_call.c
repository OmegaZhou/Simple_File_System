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
void unuse_sector(int file_section_id)
{
	int k = file_section_id % 8;
	file_section_id /= 8;
	int j = file_section_id % SECTOR_SIZE;
	int i = file_section_id / SECTOR_SIZE + BITGRAPH_START_SECTOR;
	char info[SECTOR_SIZE] = { 0 };
	get_data(i, 1, info);
	info[j] &= ~(1 << k);
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


int check_file_exist(int father_id, char* name, int* get_id)
{
	char info[SECTOR_SIZE];
	char temp[SECTOR_SIZE];
	int flag = 1;
	if (strcmp(name, "") == 0) {
		*get_id = ROOT_ID;
		return 1;
	}
	if (strcmp(name, ".")==0 || strcmp(name, "..")==0) {
		return 1;
	}
	while (father_id != -1 && flag) {
		get_data(FILE_SECTOR(father_id), 1, info);
		FILE_HEADER* header = (FILE_HEADER*)info;
		int* item_id = (int*)DATA_START_LOC(info, header->name_len);
		int num = get_item_num(header);
		for (int i = 0; i < num; ++i) {
			get_file_name(item_id[i], temp);
			if (strcmp(name, temp) == 0) {
				flag = 0;
				*get_id = item_id[i];
				return 1;
			}
		}
		father_id = header->next_id;
	}
	return 0;
}

int check_file_name(int father_id, char* name)
{
	int i;
	return check_file_exist(father_id, name, &i);
}



void get_header(FILE_HEADER* i,int id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(id), 1, info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	*i = *header;
}

int create_file(int type, int father_id, char* name)
{
	if (father_id != -1 && (check_file_name(father_id, name))) {
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
	char* name = NAME_START_LOC(info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	if (header->type == DIR_TYPE) {
		printf("/");
	}
	printf("%s", name);
}

void get_dir_trace(int id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(id), 1, info);
	char* name = NAME_START_LOC(info);;
	FILE_HEADER* header = (FILE_HEADER*)info;
	if (header->father_id != -1) {
		get_dir_trace(header->father_id);
	}
	if (header->father_id != ROOT_ID) {
		printf("/");
	}
	printf("%s", name);
}

int divide_dir_para(char* info,char** para)
{
	int flag = 0;
	int size = 0;
	para[size++] = info;
	for (int i = 0; info[i] != '\0'; ++i) {
		if (info[i] == '/') {
			flag = 1;
			info[i] = '\0';
			continue;
		}
		if (flag) {
			para[size++] = &info[i];
			flag = 0;
		}
	}
	return size;
}

int jump_to(char* location)
{
	char copy[SECTOR_SIZE] = { 0 };
	char info[SECTOR_SIZE];
	char name[SECTOR_SIZE];
	int temp_id = now_dir_id;
	int id = now_dir_id;

	char* para[SECTOR_SIZE];
	for (int i = 0; location[i] != '\0'; ++i) {
		copy[i] = location[i];
	}
	int  size = divide_dir_para(copy, para);


	for (int j = 0; j < size; ++j) {
		id = now_dir_id;
		FILE_HEADER header;
		get_header(&header, id);
		if (strcmp(para[j], ".") == 0) {
			continue;
		}
		if (strcmp(para[j], "..") == 0) {
			if (header.father_id != -1) {
				now_dir_id = header.father_id;
				continue;
			}
		}
		int get_id;
		if (check_file_exist(id, para[j], &get_id)) {
			FILE_HEADER temp;
			get_header(&temp, get_id);
			if (temp.type != DIR_TYPE) {
				now_dir_id = temp_id;
				return 1;
			}
			now_dir_id = get_id;
		} else {
			now_dir_id = temp_id;
			return 2;
		}
	}
	return 0;
}

void remove_from_father(int file_id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(file_id), 1, info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	int id = header->father_id;
	while (id != -1) {
		get_data(FILE_SECTOR(id), 1, info);
		header = (FILE_HEADER*)info;
		int* item_id = (int*)DATA_START_LOC(info, header->name_len);
		int num = get_item_num(header);
		for (int i = 0; i < num; ++i) {
			if (file_id == item_id[i]) {
				int* temp = (int*)(info + header->used_size);
				--temp;
				item_id[i] = *temp;
				header->used_size -= ID_SIZE;
				save_data(FILE_SECTOR(id), 1, info);
				return;
			}
		}
	}
}

void remove_file(int start_id)
{
	char info[SECTOR_SIZE];
	int id = start_id;
	remove_from_father(id);
	while (id != -1) {
		get_data(FILE_SECTOR(id), 1, info);
		FILE_HEADER* header = (FILE_HEADER*)info;
		if (header->type == DIR_TYPE) {
			int* item_id = (int*)DATA_START_LOC(info, header->name_len);
			int num = get_item_num(header);
			for (int i = 0; i < num; ++i) {
				remove_file(item_id[i]);
			}
		}
		unuse_sector(id);
		id = header->next_id;
	}
}
int check_remove_id(int rm_id,int now_id)
{
	int temp = now_id;
	char info[SECTOR_SIZE];
	while (temp != 0) {
		if (temp == rm_id) {
			return 0;
		}
		get_data(FILE_SECTOR(temp), 1, info);
		FILE_HEADER* header = (FILE_HEADER*)info;
		temp = header->father_id;
	}
	return 1;
}

void clear_file(int id)
{
	char info[SECTOR_SIZE];
	get_data(FILE_SECTOR(id), 1, info);
	FILE_HEADER* header = (FILE_HEADER*)info;
	header->used_size = sizeof(FILE_HEADER) + header->name_len + 1;
	id = header->next_id;
	header->next_id = -1;
	while (id != -1) {
		get_data(id, 1, info);
		header = (FILE_HEADER*)info;
		id = header->next_id;
		clear_file_sector(header->now_id);
	}
}

FILE_HEADER* open_file(int id,char* space)
{
	get_data(FILE_SECTOR(id), 1, space);
	return (FILE_HEADER*)space;
}

void write_file(char c, FILE_HEADER* file)
{
	char* temp = (char*)file;
	int id = file->now_id;
	if (file->used_size + 1 >= SECTOR_SIZE) {
		save_data(FILE_SECTOR(id), 1, temp);
		id=add_file_sector(FILE_TYPE, file->father_id, file->now_id, NAME_START_LOC(temp), file);
		get_data(FILE_SECTOR(id), 1, temp);
	}
	*(temp + file->used_size) = c;
	++file->used_size;
}
int read_file(FILE_HEADER* file, int* loc)
{
	int start = sizeof(FILE_HEADER) + file->name_len + 1;
	char* temp = (char*)file;
	if (start + *loc >= file->used_size) {
		if (file->next_id == -1) {
			return EOF;
		} else {
			get_data(FILE_SECTOR(file->next_id), 1, temp);
		}
	}
	temp = DATA_START_LOC(temp, file->name_len);
	char re = temp[*loc];
	++(*loc);
	return re;
}
void close_file(FILE_HEADER* file)
{
	save_data(FILE_SECTOR(file->now_id), 1, (char*)file);
}