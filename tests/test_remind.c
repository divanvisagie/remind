#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_OUTPUT_SIZE 4096
#define MAX_PATH_SIZE 512
#define MAX_CMD_SIZE 1024

// Colors
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define NC "\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;
static char test_home[MAX_PATH_SIZE];
static char remind_file[MAX_PATH_SIZE];
static char binary_path[MAX_PATH_SIZE] = "./bin/remind";

void pass_test(const char* test_name) {
    printf("%s" GREEN "✓ PASS" NC "\n", test_name);
    tests_passed++;
}

void fail_test(const char* test_name, const char* reason) {
    printf("%s" RED "✗ FAIL: %s" NC "\n", test_name, reason);
    tests_failed++;
}

// Execute command and capture output
int run_command(const char* cmd, char* output, size_t output_size) {
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }

    output[0] = '\0';
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strlen(output) + strlen(buffer) < output_size - 1) {
            strcat(output, buffer);
        }
    }

    int status = pclose(fp);
    return WEXITSTATUS(status);
}

// Check if file exists
int file_exists(const char* path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

// Check if file is empty
int file_is_empty(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return st.st_size == 0;
}

// Count lines in file
int count_lines(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) return -1;

    int lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        lines++;
    }
    fclose(fp);
    return lines;
}

// Check if file contains string
int file_contains(const char* path, const char* search) {
    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, search)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// Create temporary directory and set up test environment
int setup_test_env() {
    // Create temporary directory
    snprintf(test_home, sizeof(test_home), "/tmp/remind_test_c_%d", getpid());
    if (mkdir(test_home, 0755) != 0) {
        perror("mkdir test_home");
        return -1;
    }

    // Set HOME environment variable
    if (setenv("HOME", test_home, 1) != 0) {
        perror("setenv HOME");
        return -1;
    }

    // Set remind file path
    snprintf(remind_file, sizeof(remind_file), "%s/.local/state/remind/reminders", test_home);

    return 0;
}

// Clean up test environment
void cleanup_test_env() {
    char cmd[MAX_CMD_SIZE];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", test_home);
    system(cmd);
}

// Test 1: Empty check creates file
void test_empty_file_behavior() {
    printf("Test 1: Empty file behavior\n");

    char cmd[MAX_CMD_SIZE];
    char output[MAX_OUTPUT_SIZE];

    snprintf(cmd, sizeof(cmd), "%s -c 2>&1", binary_path);
    run_command(cmd, output, sizeof(output));

    if (strlen(output) == 0 && file_exists(remind_file) && file_is_empty(remind_file) == 1) {
        pass_test("");
    } else {
        fail_test("", "Should create empty file and produce no output");
    }
}

// Test 2: Add first reminder
void test_add_first_reminder() {
    printf("Test 2: Add first reminder\n");

    char cmd[MAX_CMD_SIZE];
    snprintf(cmd, sizeof(cmd), "%s -a \"Buy groceries\"", binary_path);
    system(cmd);

    if (file_exists(remind_file) && file_contains(remind_file, "Buy groceries")) {
        pass_test("");
    } else {
        fail_test("", "Should add reminder to file");
    }
}

// Test 3: Add second reminder
void test_add_second_reminder() {
    printf("Test 3: Add second reminder\n");

    char cmd[MAX_CMD_SIZE];
    snprintf(cmd, sizeof(cmd), "%s -a \"Call mom\"", binary_path);
    system(cmd);

    int line_count = count_lines(remind_file);
    if (line_count == 2 && file_contains(remind_file, "Call mom")) {
        pass_test("");
    } else {
        fail_test("", "Should have 2 reminders in file");
    }
}

// Test 4: Display reminders
void test_display_reminders() {
    printf("Test 4: Display reminders\n");

    char cmd[MAX_CMD_SIZE];
    char output[MAX_OUTPUT_SIZE];

    snprintf(cmd, sizeof(cmd), "%s -c", binary_path);
    run_command(cmd, output, sizeof(output));

    if (strstr(output, "Reminders") &&
        strstr(output, "1. Buy groceries") &&
        strstr(output, "2. Call mom")) {
        pass_test("");
    } else {
        fail_test("", "Should display numbered reminders with header");
    }
}

// Test 5: Delete reminder
void test_delete_reminder() {
    printf("Test 5: Delete reminder\n");

    char cmd[MAX_CMD_SIZE];
    snprintf(cmd, sizeof(cmd), "%s -d 1", binary_path);
    system(cmd);

    int line_count = count_lines(remind_file);
    if (line_count == 1 &&
        file_contains(remind_file, "Call mom") &&
        !file_contains(remind_file, "Buy groceries")) {
        pass_test("");
    } else {
        fail_test("", "Should delete first reminder");
    }
}

// Test 6: Display after deletion
void test_display_after_deletion() {
    printf("Test 6: Display after deletion\n");

    char cmd[MAX_CMD_SIZE];
    char output[MAX_OUTPUT_SIZE];

    snprintf(cmd, sizeof(cmd), "%s -c", binary_path);
    run_command(cmd, output, sizeof(output));

    if (strstr(output, "1. Call mom") && !strstr(output, "Buy groceries")) {
        pass_test("");
    } else {
        fail_test("", "Should show remaining reminder with correct numbering");
    }
}

// Test 7: Help with -h flag
void test_help_short_flag() {
    printf("Test 7: Help with -h flag\n");

    char cmd[MAX_CMD_SIZE];
    char output[MAX_OUTPUT_SIZE];

    snprintf(cmd, sizeof(cmd), "%s -h", binary_path);
    run_command(cmd, output, sizeof(output));

    if (strstr(output, "remind - simple reminder manager") &&
        strstr(output, "USAGE:") &&
        strstr(output, "OPTIONS:")) {
        pass_test("");
    } else {
        fail_test("", "Should display help message with -h flag");
    }
}

// Test 8: Help with --help flag
void test_help_long_flag() {
    printf("Test 8: Help with --help flag\n");

    char cmd[MAX_CMD_SIZE];
    char output[MAX_OUTPUT_SIZE];

    snprintf(cmd, sizeof(cmd), "%s --help", binary_path);
    run_command(cmd, output, sizeof(output));

    if (strstr(output, "remind - simple reminder manager") &&
        strstr(output, "EXAMPLES:") &&
        strstr(output, "remind -a")) {
        pass_test("");
    } else {
        fail_test("", "Should display help message with --help flag");
    }
}

int main(int argc, char* argv[]) {
    printf("Simple Functional Tests for Remind (C Version)\n");
    printf("==============================================\n");

    // Check if binary exists
    if (!file_exists(binary_path)) {
        printf(RED "Error: %s not found. Run 'make' first." NC "\n", binary_path);
        return 1;
    }

    // Setup test environment
    if (setup_test_env() != 0) {
        printf(RED "Failed to setup test environment" NC "\n");
        return 1;
    }

    // Run tests
    test_empty_file_behavior();
    test_add_first_reminder();
    test_add_second_reminder();
    test_display_reminders();
    test_delete_reminder();
    test_display_after_deletion();
    test_help_short_flag();
    test_help_long_flag();

    // Cleanup
    cleanup_test_env();

    // Summary
    printf("\nTest Summary:\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf(GREEN "All tests passed!" NC "\n");
        return 0;
    } else {
        printf(RED "Some tests failed." NC "\n");
        return 1;
    }
}
