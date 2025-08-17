#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

typedef struct {
	bool check;
	bool add;
	bool edit;
} Args;

char* getenv_value(char* name) {
	char* full_val = getenv(name);
	if (full_val != NULL) {
		char* value = strchr(full_val, '=');
		if (value != NULL) {
			return value + 1;
		} else {
			return full_val;
		}
	}
	return NULL	;
}

bool print_header() {
	printf("##################################\n");
	printf("#             Reminders           \n");
	printf("##################################\n");
	return true;
}

void check_reminders(char* file_path) {

	FILE *f = fopen(file_path, "r");
	if (!f) {
		perror("fopen");
		return;
	}

	char line[1024];
	bool has_printed_header = false;
	while (fgets(line, sizeof(line), f)) {
		if (!has_printed_header)	{
			has_printed_header = print_header();
		}
		printf("%s\n", line);
	}

	if (has_printed_header) {
		printf("##################################\n");
	}
}

int main(int argc, char** argv) {
	Args *args  = malloc(sizeof(Args));
	args->check = false;
	for (int i =0; i < argc; i++) {
		if (strcmp("-c", argv[i]) == 0) {
			args->check = true;
		}
	}

	char* file_path = getenv_value("HOME");
	strncat(file_path,"/.local/state/remind/reminders", sizeof(file_path) - strlen(file_path) - 1);

	if (args->check) {
		check_reminders(file_path);
	} else {
		char path[PATH_MAX];
    	snprintf(path, sizeof(path), "%s", file_path);

    	char *const argv[] = {"nvim", path, NULL};
    	execvp("nvim", argv);

    	perror("execvp");
		return 1;
	}
	free(args);
	return 0;
}
