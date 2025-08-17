#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct {
    bool check;
    char* add;
    bool edit;
    int  delete;   // line number to delete, -1 = none
} Args;

bool print_header(void) {
    printf("###################################\n");
    printf("#             Reminders           #\n");
    printf("###################################\n\n");
    return true;
}

void check_reminders(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    if (!f) {
        perror("fopen");
        return;
    }

    char line[1024];
	int lc = 1;
    bool has_printed_header = false;
    while (fgets(line, sizeof(line), f)) {
        if (!has_printed_header) {
            has_printed_header = print_header();
        }
        printf("%d. %s",lc, line);
		lc++;
    }
    fclose(f);

    if (has_printed_header) {
        printf("\n");
    }
}

void delete_line(const char *file_path, int target_line) {
    FILE *f = fopen(file_path, "r");
    if (!f) {
        perror("fopen read");
        return;
    }

    // Get the file size so that we can use it in malloc
    struct stat st;
    if (stat(file_path, &st) != 0) {
        perror("stat");
        fclose(f);
        return;
    }
    long size = st.st_size;

    char *buf = malloc(size + 1);
    if (!buf) {
        perror("malloc");
        fclose(f);
        return;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    // Open for writing (truncate)
    f = fopen(file_path, "w");
    if (!f) {
        perror("fopen write");
        free(buf);
        return;
    }

    int lineno = 1;
    char *line = strtok(buf, "\n");
    while (line) {
        if (lineno != target_line) {
            fprintf(f, "%s\n", line);
        }
        line = strtok(NULL, "\n");
        lineno++;
    }

    free(buf);
    fclose(f);
}

void add_reminder(const char *file_path, const char *text) {
    FILE *f = fopen(file_path, "a"); 
    if (!f) {
        perror("fopen append");
        return;
    }

    fprintf(f, "%s\n", text);
    fclose(f);
}

int main(int argc, char **argv) {
    Args args = {0};
    args.delete = -1;
	args.add = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp("-c", argv[i]) == 0) {
            args.check = true;
        } 
		else if (strcmp("-a", argv[i]) == 0) {
			if (i + 1 >= argc) {
                fprintf(stderr, "Please supply some text after the -a\n");
                exit(1);
			}
			args.add = argv[i + 1];
            i++; // skip over the number
		}
		else if (strcmp("-d", argv[i]) == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Please supply a line number after -d\n");
                exit(1);
            }
            char *endptr;
            long line_n = strtol(argv[i + 1], &endptr, 10);
            if (*endptr != '\0' || line_n < 1) {
                fprintf(stderr, "Invalid line number: %s\n", argv[i + 1]);
                exit(1);
            }
            args.delete = (int) line_n;
            i++; // skip over the number
        }
    }

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path),
             "%s/.local/state/remind/reminders",
             getenv("HOME"));
    if (args.check) {
        check_reminders(file_path);
    } else if (args.delete >= 0) {
		delete_line(file_path, args.delete);
	} else if (args.add != NULL) {
		add_reminder(file_path, args.add);
	} else {
        char *const exec_args[] = {"nvim", file_path, NULL};
        execvp("nvim", exec_args);
        perror("execvp"); // only if exec fails
        return 1;
    }

    return 0;
}

