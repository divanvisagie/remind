#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>

#define NUMBER_SPACING 2
#define MAX_LINES 1024
#define MAX_LINE_LENGTH 1024
#define MAX_LINE_CHARACTER_LENGTH 1024

typedef struct {
    bool check;
    char* add;
    bool edit;
    int  delete;   // Line number to delete, -1 = none
} Args;

/// Prints a single character on repeat a specific amount of times
void print_for(char *c, int times) {
   for (int i=0; i < times; i++) printf("%s", c);
}

bool print_header(int width) {
    const int title_length = 9;
    if (width < title_length + 2) {
        width = title_length + 4;
    }
    int space_length = (width - title_length - 2) / 2;
    char *title = "Reminders";
    print_for("#", width); printf("\n#");
    print_for(" ", space_length); printf("%s", title); print_for(" ", space_length); printf("#\n");
    print_for("#", width); printf("\n");
    return true;
}

/// Ensures the directory for the reminders file exists
void ensure_remind_dir(const char *file_path) {
    char dir_path[PATH_MAX];
    strcpy(dir_path, file_path);

    // Find the last slash to get directory path
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';  // Terminate string at last slash

        // Use system command to create directory recursively
        char mkdir_cmd[PATH_MAX + 20];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", dir_path);
        system(mkdir_cmd);
    }
}

/// Checks the reminders in the file and prints them out
void check_reminders(const char *file_path) {
    // Ensure directory exists first
    ensure_remind_dir(file_path);

    FILE *f = fopen(file_path, "r");
    if (!f) {
        // Create empty file if it doesn't exist
        f = fopen(file_path, "w");
        if (f) {
            fclose(f);
        }
        // No reminders to show yet
        return;
    }

    int longest_length = 0;
    char line[MAX_LINE_CHARACTER_LENGTH];
    char *lines[MAX_LINES];
	int lc = 1;
    bool has_printed_header = false;
    while (fgets(line, sizeof(line), f) && lc <= MAX_LINES) {
        int length = strlen(line);
        if (length > longest_length) longest_length = length;
        lines[lc - 1] = strdup(line);
		lc++;
    }

    // Check if we hit the limit before closing file
    bool hit_limit = !feof(f);
    fclose(f);


    char num_string[32];
    sprintf(num_string, "%d", lc);
    int longest_number_length = strlen(num_string) + NUMBER_SPACING;

    for (int i =0; i < lc -1; i++) {
        if (!has_printed_header) {
            /* The header feature is supposed to be used to grab attention
             * when the user integrates the program into their shell startup.
             * Logically they only want to see it if there are items on the list
             * which is why we keep this in the loop.
             */
            has_printed_header = print_header(longest_length + longest_number_length);
        }
        printf("%d. %s",i + 1, lines[i]);
        free(lines[i]);
    }

    if (hit_limit) {
        fprintf(stderr, "\nWarning: File has more than %d lines. Only showing first %d lines.\n", MAX_LINES, MAX_LINES);
    }
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
            i++; // Skip over because we already read it above
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
            i++;
        }
    }

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path),
             "%s/.local/state/remind/reminders",
             getenv("HOME"));
    if (args.check) {
        check_reminders(file_path);
    } else if (args.delete >= 0) {
		ensure_remind_dir(file_path);
		delete_line(file_path, args.delete);
	} else if (args.add != NULL) {
		ensure_remind_dir(file_path);
		add_reminder(file_path, args.add);
	} else {
		ensure_remind_dir(file_path);
	    char *editor = getenv("EDITOR");
		if (!editor) {
			editor = "vi";
		}
		char *const exec_args[] = {editor, file_path, NULL};
        execvp(editor, exec_args);
        perror("execvp");
        return 1;
    }

    return 0;
}
