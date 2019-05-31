#include "file_sys_call.h"
#include <stdio.h>
#include <string.h>
int ls(int argc, char* argv[])
{
	char info[SECTOR_SIZE];
	int temp = now_dir_id;
	if (argc > 1) {
		for (int j = 1; j < argc; ++j) {
			if (argc > 2) {
				printf("%s:   ", argv[j]);
			}
			int status=jump_to(argv[j]);
			if (status == 2) {
				printf("ls: cannot access \'%s\' : No such file or directory\n", argv[j]);
			} else if (status == 1) {
				printf("%s", argv[j]);
			} else {
				int id = now_dir_id;
				while (id != -1) {
					get_data(FILE_SECTOR(id), 1, info);
					FILE_HEADER* header = (FILE_HEADER*)info;
					int* item_id = (int*)DATA_START_LOC(info, header->name_len);
					int num = get_item_num(header);
					for (int i = 0; i < num; ++i) {
						print_file_name(item_id[i]);
						printf(" ");
					}
					id = header->next_id;
				}
				printf("\n");
			}
			now_dir_id = temp;
		}
	} else {
		int id = now_dir_id;
		while (id != -1) {
			get_data(FILE_SECTOR(id), 1, info);
			FILE_HEADER* header = (FILE_HEADER*)info;
			int* item_id = (int*)DATA_START_LOC(info, header->name_len);
			int num = get_item_num(header);
			for (int i = 0; i < num; ++i) {
				print_file_name(item_id[i]);
				printf(" ");
			}
			id = header->next_id;
		}
		printf("\n");
	}
	
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
	int temp = now_dir_id;
	for (int i = 1; i < argc; ++i) {
		int c = -1;
		int size = strlen(argv[i]);
		for (int j = size - 1; j >= 0; --j) {
			if (argv[i][j] == '/') {
				argv[i][j] = '\0';
				c = j;
				break;
			}
		}
		if (c != -1) {
			int flag=jump_to(argv[i]);
			if (flag == 1) {
				if (c != -1) {
					argv[i][c] = '/';
				}
				printf("mkdir: %s: Not a directory\n", argv[i]);
				continue;
			} else if (flag == 2) {
				if (c != -1) {
					argv[i][c] = '/';
				}
				printf("mkdir: %s: No such file or directory\n", argv[i]);
				continue;
			}
		}
		if (!create_file(DIR_TYPE, now_dir_id, &argv[i][c + 1])) {
			if (c!=-1) {
				argv[i][c] = '/';
			}
			printf("mkdir: cannot create directory \'%s\': File exists\n",argv[i]);
		}
		now_dir_id = temp;
	}
	return 0;
}

int cd(int argc, char* argv[])
{
	if (argc <= 1) {
		now_dir_id = ROOT_ID;
		return 0;
	}
	int flag = jump_to(argv[1]);
	if (flag == 1) {
		printf("cd: %s: Not a directory\n", argv[1]);
	} else if (flag == 2) {
		printf("cd: %s: No such file or directory\n", argv[1]);
	}
	return 0;
}