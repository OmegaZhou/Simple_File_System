#include "file_sys_call.h"
#include <stdio.h>
#include <string.h>
int ls(int argc, char* argv[])
{
	char info[SECTOR_SIZE];
	int temp = now_dir_id;
	if (argc > 1) {
		cd(argc, argv);
	}
	int id = now_dir_id;
	while (id != -1) {
		get_data(FILE_SECTOR(id), 1, info);
		FILE_HEADER* header = (FILE_HEADER*)info;
		int* item_id = (int*)DATA_START_LOC(info,header->name_len);
		int num = get_item_num(header);
		for (int i = 0; i < num; ++i) {
			print_file_name(item_id[i]);
			printf(" ");
		}
		id = header->next_id;
	}
	printf("\n");
	now_dir_id = temp;
	return 0;
}

int mkfs(int argc, char* argv[])
{
	clear_file_sector(FILE_SECTOR(ROOT_ID));
	clear_bitgraph();
	create_file(DIR_TYPE, -1, "");
	return 0;
}

int pwd(int argc, char* argv[])
{
	get_dir_trace(now_dir_id);
	printf("\n");
	return 0;
}

int mkdir(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i) {
		char* para = argv[i];
		int flag = 1;
		for (int j = 0; argv[1][j] != '\0';++j) {
			if (argv[1][j] == '/') {
				if (j == 0) {
					++para;
				} else if (argv[1][j + 1] == '\0') {
					argv[1][j] = '\0';
				} else {
					printf("mkdir: cannot create directory \'%s\': File name can't include\'/\'\n", argv[i]);
					flag = 0;
					break;
				}
			}
		}
		if (flag == 0) {
			continue;
		}
		if (!create_file(DIR_TYPE, now_dir_id, argv[i])) {
			printf("mkdir: cannot create directory \'%s\': File exists\n",argv[i]);
		}
	}
	return 0;
}

int cd(int argc, char* argv[])
{
	if (argc <= 1) {
		now_dir_id = ROOT_ID;
		return 0;
	}
	char info[SECTOR_SIZE];
	char name[SECTOR_SIZE];
	int temp_id = now_dir_id;
	int id = now_dir_id;
	
	char* para[SECTOR_SIZE];
	int  size = divide_dir_para(argv[1], para);
	

	for (int j = 0; j < size; ++j) {
		id = now_dir_id;
		FILE_HEADER header;
		get_header(&header, id);
		if (strcmp(para[j], ".")==0) {
			continue;
		}
		if (strcmp(para[j], "..") == 0) {
			if (header.father_id != -1) {
				now_dir_id = header.father_id;
				continue;
			}
		}
		int get_id;
		if (check_file_exist(id, para[j],&get_id)) {
			FILE_HEADER temp;
			get_header(&temp, get_id);
			if (temp.type != DIR_TYPE) {
				printf("cd: %s: Not a directory\n", para[j]);
				now_dir_id = temp_id;
				break;
			}
			now_dir_id = get_id;
		} else {
			printf("cd: %s: No such file or directory\n", para[j]);
			now_dir_id = temp_id;
			break;
		}
	}
	return 0;
}