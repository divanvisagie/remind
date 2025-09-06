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

typedef enum {
    ACTION_CHECK,
    ACTION_ADD,
    ACTION_DELETE,
    ACTION_EDIT,
    ACTION_HELP
} Action;

typedef struct {
    const char* flag;
    Action action;
    bool needs_arg;
} FlagMapping;

void print_help() {
    printf("remind - simple reminder manager\n\n");
    printf("USAGE:\n");
    printf("    remind [OPTIONS]\n\n");
    printf("OPTIONS:\n");
    printf("    -c              Check reminders. Prints the current list of reminders.\n");
    printf("    -a TEXT         Add a new reminder line containing TEXT.\n");
    printf("    -d N            Delete reminder at line number N (1-based).\n");
    printf("    -h, --help      Show this help message.\n");
    printf("    (no options)    Open the reminders file in $EDITOR for manual editing.\n\n");
    printf("EXAMPLES:\n");
    printf("    remind -a \"Buy milk\"    Add a reminder\n");
    printf("    remind -c              List all reminders\n");
    printf("    remind -d 2            Delete the second reminder\n");
    printf("    remind                 Edit reminders manually\n\n");
    printf("FILES:\n");
    printf("    $HOME/.local/state/remind/reminders    Storage location of reminders\n\n");
    printf("For more information, see remind(1).\n");
}

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

    FlagMapping flags[] = {
        {"-c", ACTION_CHECK, false},
        {"-a", ACTION_ADD, true},
        {"-d", ACTION_DELETE, true},
        {"-h", ACTION_HELP, false},
        {"--help", ACTION_HELP, false}
    };
    const int num_flags = sizeof(flags) / sizeof(flags[0]);

    for (int i = 1; i < argc; i++) {
        bool flag_found = false;
        
        for (int j = 0; j < num_flags; j++) {
            if (strcmp(argv[i], flags[j].flag) == 0) {
                flag_found = true;
                
                switch (flags[j].action) {
                    case ACTION_CHECK:
                        args.check = true;
                        break;
                        
                    case ACTION_ADD:
                        if (i + 1 >= argc) {
                            fprintf(stderr, "Please supply some text after the -a\n");
                            exit(1);
                        }
                        args.add = argv[i + 1];
                        i++; // Skip over because we already read it above
                        break;
                        
                    case ACTION_DELETE:
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
                        break;
                        
                    case ACTION_HELP:
                        args.check = false; // Clear other flags
                        args.add = NULL;
                        args.delete = -1;
                        break;
                        
                    case ACTION_EDIT:
                        // Reserved for future use
                        break;
                }
                break;
            }
        }
        
        if (!flag_found) {
            fprintf(stderr, "Unknown flag: %s\n", argv[i]);
            exit(1);
        }
    }

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path),
             "%s/.local/state/remind/reminders",
             getenv("HOME"));

    // Determine which action to take
    Action chosen_action = ACTION_EDIT; // Default action
    
    // Check if help was requested (check the original arguments)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            chosen_action = ACTION_HELP;
            break;
        }
    }
    
    if (chosen_action != ACTION_HELP) {
        if (args.check) {
            chosen_action = ACTION_CHECK;
        } else if (args.delete >= 0) {
            chosen_action = ACTION_DELETE;
        } else if (args.add != NULL) {
            chosen_action = ACTION_ADD;
        }
    }

    switch (chosen_action) {
        case ACTION_CHECK:
            check_reminders(file_path);
            break;
            
        case ACTION_DELETE:
            ensure_remind_dir(file_path);
            delete_line(file_path, args.delete);
            break;
            
        case ACTION_ADD:
            ensure_remind_dir(file_path);
            add_reminder(file_path, args.add);
            break;
            
        case ACTION_HELP:
            print_help();
            break;
            
        case ACTION_EDIT:
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
