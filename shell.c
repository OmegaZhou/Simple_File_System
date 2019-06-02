#include "shell.h"
#include "file_sys_command.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define MAX_NAME_SIZE 12
struct Command
{
	char name[MAX_NAME_SIZE];
	main_func func;
};
struct Command command_table[] = { {"ls",ls},{"pwd",pwd },{"mkfs",mkfs},{"mkdir",mkdir},{"cd",cd},{"rm",rm},{"mkfl",mkfl},{"wrfl",wrfl},{"cat",cat} };
void get_command()
{
	static char command[512] = { 0 };
	static char* argv[PARAM_SIZE];
	char c;
	int size = 0;
	while ((c = getchar()) != '\n') {
		command[size++] = c;
	}
	command[size++] = '\0';
	main_func command_pointer;
	int command_len = strlen(command);
	int start_flag = 0;
	int end_flag = 0;
	int argc = 0;
	for (int i = 0; i < command_len; ++i) {
		if (!isspace(command[i])) {
			if (start_flag == 0) {
				start_flag = 1;
				end_flag = 0;
				argv[argc] = command + i;
				++argc;
			}
		} else {
			if (end_flag == 0) {
				command[i] = '\0';
				start_flag = 0;
				end_flag = 1;
			}
		}
	}
	int command_table_size = sizeof(command_table) / sizeof(struct Command);
	for (int i = 0; i < command_table_size; ++i) {
		if (strcmp("", argv[0]) == 0) {
			return;
		}
		if (strcmp(command_table[i].name, argv[0]) == 0) {
			command_pointer = command_table[i].func;
			(*command_pointer)(argc, argv);
			return;
		}
	}
	printf("Command not found\n");
}

void init_shell()
{
	for (;;) {
		printf("Zhou>");
		get_command();
	}
}