#include "file_sys_call.h"
#include <stdio.h>
int ls(int argc, char* argv[])
{
	char info[SECTOR_SIZE];
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
	return 0;
}

int init_file_sys(int argc, char* argv[])
{
	clear_file_sector(FILE_SECTOR(ROOT_ID));
	clear_bitgraph();
	create_file(DIR_TYPE, -1, "");
	return 0;
}

int pwd(int argc, char* argv[])
{
	print_file_name(now_dir_id);
	return 0;
}

int mkdir(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i) {
		if (!create_file(DIR_TYPE, now_dir_id, argv[i])) {
			printf("mkdir: cannot create directory \'%s\': File exists\n",argv[i]);
		}
	}
}

void test()
{
	init_file_sys(1, NULL);
	char* i[6];
	i[1] = "a";
	i[2] = "a";
	i[3] = "af";
	i[4] = "af";
	i[5] = "t";
	mkdir(6, i);
	ls(1, NULL);
}