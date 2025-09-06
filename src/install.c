#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <pwd.h>

#define MAX_PATH 4096
#define MAX_CMD 8192

typedef enum {
    LOG_INFO,
    LOG_SUCCESS, 
    LOG_WARNING,
    LOG_ERROR
} log_level_t;

void print_log(log_level_t level, const char* message) {
    const char* colors[] = {
        "\033[0;34m[INFO]\033[0m",     // BLUE
        "\033[0;32m[SUCCESS]\033[0m",  // GREEN
        "\033[1;33m[WARNING]\033[0m",  // YELLOW
        "\033[0;31m[ERROR]\033[0m"     // RED
    };
    printf("%s %s\n", colors[level], message);
}

int file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

int dir_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int run_command(const char* cmd) {
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int has_sudo_privileges() {
    return system("sudo -n true 2>/dev/null") == 0;
}

int is_root() {
    return getuid() == 0;
}

int create_directory(const char* path, int use_sudo) {
    char cmd[MAX_CMD];
    if (use_sudo) {
        snprintf(cmd, sizeof(cmd), "sudo mkdir -p \"%s\"", path);
    } else {
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path);
    }
    return run_command(cmd);
}

int copy_file(const char* src, const char* dest, int use_sudo) {
    char cmd[MAX_CMD];
    if (use_sudo) {
        snprintf(cmd, sizeof(cmd), "sudo cp \"%s\" \"%s\"", src, dest);
    } else {
        snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", src, dest);
    }
    return run_command(cmd);
}

int make_executable(const char* path, int use_sudo) {
    char cmd[MAX_CMD];
    if (use_sudo) {
        snprintf(cmd, sizeof(cmd), "sudo chmod +x \"%s\"", path);
    } else {
        snprintf(cmd, sizeof(cmd), "chmod +x \"%s\"", path);
    }
    return run_command(cmd);
}

int in_path(const char* dir) {
    const char* path_env = getenv("PATH");
    if (!path_env) return 0;
    
    char* path_copy = strdup(path_env);
    char* token = strtok(path_copy, ":");
    
    while (token) {
        if (strcmp(token, dir) == 0) {
            free(path_copy);
            return 1;
        }
        token = strtok(NULL, ":");
    }
    
    free(path_copy);
    return 0;
}

void get_home_dir(char* home_dir, size_t size) {
    const char* home = getenv("HOME");
    if (home) {
        strncpy(home_dir, home, size - 1);
        home_dir[size - 1] = '\0';
    } else {
        struct passwd* pw = getpwuid(getuid());
        strncpy(home_dir, pw->pw_dir, size - 1);
        home_dir[size - 1] = '\0';
    }
}

void get_shell_config_file(char* config_file, size_t size) {
    char home_dir[MAX_PATH];
    get_home_dir(home_dir, sizeof(home_dir));
    
    const char* shell = getenv("SHELL");
    if (!shell) {
        snprintf(config_file, size, "%s/.profile", home_dir);
        return;
    }
    
    const char* shell_name = strrchr(shell, '/');
    shell_name = shell_name ? shell_name + 1 : shell;
    
    if (strcmp(shell_name, "bash") == 0) {
        snprintf(config_file, size, "%s/.bashrc", home_dir);
    } else if (strcmp(shell_name, "zsh") == 0) {
        snprintf(config_file, size, "%s/.zshrc", home_dir);
    } else if (strcmp(shell_name, "fish") == 0) {
        snprintf(config_file, size, "%s/.config/fish/config.fish", home_dir);
    } else {
        snprintf(config_file, size, "%s/.profile", home_dir);
    }
}

int add_to_path(const char* dir) {
    char config_file[MAX_PATH];
    get_shell_config_file(config_file, sizeof(config_file));
    
    const char* shell = getenv("SHELL");
    const char* shell_name = shell ? strrchr(shell, '/') : NULL;
    shell_name = shell_name ? shell_name + 1 : "sh";
    
    FILE* file = fopen(config_file, "a");
    if (!file) {
        print_log(LOG_ERROR, "Failed to open shell configuration file");
        return 0;
    }
    
    if (strcmp(shell_name, "fish") == 0) {
        fprintf(file, "set -gx PATH %s $PATH\n", dir);
    } else {
        fprintf(file, "export PATH=\"%s:$PATH\"\n", dir);
    }
    
    fclose(file);
    return 1;
}

int main() {
    char home_dir[MAX_PATH];
    char bin_dir[MAX_PATH];
    char man_dir[MAX_PATH];
    char binary_path[MAX_PATH];
    char target_binary[MAX_PATH];
    char target_man[MAX_PATH];
    
    get_home_dir(home_dir, sizeof(home_dir));
    
    print_log(LOG_INFO, "Smart installation script for remind");
    
    // Check if we're in the project root
    if (!file_exists("src/main.c") || !file_exists("remind.1")) {
        print_log(LOG_ERROR, "This script must be run from the remind project root directory");
        return 1;
    }
    
    // Build the project first
    print_log(LOG_INFO, "Building remind...");
    if (!run_command("make main")) {
        print_log(LOG_ERROR, "Failed to build remind");
        return 1;
    }
    
    // Check if binary exists
    if (!file_exists("bin/remind")) {
        print_log(LOG_ERROR, "Binary not found at bin/remind");
        return 1;
    }
    
    // Detect privileges
    int root = is_root();
    int sudo = has_sudo_privileges();
    int use_sudo = 0;
    int system_install = 0;
    
    if (root) {
        print_log(LOG_INFO, "Running as root");
        system_install = 1;
    } else if (sudo) {
        print_log(LOG_INFO, "User has sudo privileges");
        
        // Ask user preference
        printf("\nYou have administrative privileges. Choose installation type:\n");
        printf("  1) System-wide installation (/usr/local/bin) - recommended\n");
        printf("  2) User-local installation (~/.local/bin)\n\n");
        
        char choice[10];
        printf("Enter choice [1]: ");
        fflush(stdout);
        
        if (fgets(choice, sizeof(choice), stdin) && choice[0] == '2') {
            system_install = 0;
        } else {
            system_install = 1;
            use_sudo = 1;
        }
    } else {
        print_log(LOG_INFO, "Running as regular user");
        system_install = 0;
    }
    
    // Set installation paths
    if (system_install) {
        strcpy(bin_dir, "/usr/local/bin");
        strcpy(man_dir, "/usr/local/share/man/man1");
        print_log(LOG_INFO, "Installing remind (system-wide) to: /usr/local/bin");
    } else {
        snprintf(bin_dir, sizeof(bin_dir), "%s/.local/bin", home_dir);
        snprintf(man_dir, sizeof(man_dir), "%s/.local/share/man/man1", home_dir);
        print_log(LOG_INFO, "Installing remind (user-local) to: ~/.local/bin");
    }
    
    // Create target paths
    snprintf(target_binary, sizeof(target_binary), "%s/remind", bin_dir);
    snprintf(target_man, sizeof(target_man), "%s/remind.1", man_dir);
    
    // Create directories and install
    if (!create_directory(bin_dir, use_sudo) || !create_directory(man_dir, use_sudo)) {
        print_log(LOG_ERROR, "Failed to create installation directories");
        return 1;
    }
    
    if (!copy_file("bin/remind", target_binary, use_sudo) ||
        !copy_file("remind.1", target_man, use_sudo)) {
        print_log(LOG_ERROR, "Failed to copy files");
        return 1;
    }
    
    if (!make_executable(target_binary, use_sudo)) {
        print_log(LOG_ERROR, "Failed to make binary executable");
        return 1;
    }
    
    print_log(LOG_SUCCESS, "Installation completed!");
    
    // Check and configure PATH for user installations
    if (!system_install) {
        print_log(LOG_INFO, "Checking PATH configuration...");
        
        if (in_path(bin_dir)) {
            print_log(LOG_SUCCESS, "~/.local/bin is already in your PATH");
        } else {
            print_log(LOG_WARNING, "~/.local/bin is not in your PATH");
            
            char config_file[MAX_PATH];
            get_shell_config_file(config_file, sizeof(config_file));
            
            const char* shell = getenv("SHELL");
            const char* shell_name = shell ? strrchr(shell, '/') : NULL;
            shell_name = shell_name ? shell_name + 1 : "sh";
            
            printf("\nTo add ~/.local/bin to your PATH, you can:\n");
            printf("  1) Run this command now (recommended):\n");
            if (strcmp(shell_name, "fish") == 0) {
                printf("     fish_add_path ~/.local/bin\n");
            } else {
                printf("     echo 'export PATH=\"$HOME/.local/bin:$PATH\"' >> %s\n", config_file);
            }
            printf("  2) Add it manually to your shell configuration\n");
            printf("  3) Run 'remind' with full path: ~/.local/bin/remind\n\n");
            
            char add_path[10];
            printf("Would you like to add ~/.local/bin to your PATH now? [y/N]: ");
            fflush(stdout);
            
            if (fgets(add_path, sizeof(add_path), stdin) && 
                (add_path[0] == 'y' || add_path[0] == 'Y')) {
                
                if (add_to_path(bin_dir)) {
                    print_log(LOG_SUCCESS, "Added ~/.local/bin to PATH");
                    printf("Please restart your terminal or run: source %s\n", config_file);
                } else {
                    print_log(LOG_ERROR, "Failed to add ~/.local/bin to PATH");
                }
            }
        }
    }
    
    // Test the installation
    printf("\n");
    print_log(LOG_INFO, "Testing installation...");
    
    if (file_exists(target_binary)) {
        char msg[MAX_PATH + 50];
        snprintf(msg, sizeof(msg), "remind is installed at %s", target_binary);
        print_log(LOG_SUCCESS, msg);
        
        if (system_install) {
            if (system("command -v remind >/dev/null 2>&1") == 0) {
                print_log(LOG_SUCCESS, "remind is available in PATH");
                print_log(LOG_INFO, "You can now run: remind");
            } else {
                print_log(LOG_WARNING, "remind binary exists but may not be in PATH");
                print_log(LOG_INFO, "Try running: hash -r && remind");
            }
        } else if (in_path(bin_dir)) {
            print_log(LOG_SUCCESS, "remind should be available in PATH");
            print_log(LOG_INFO, "You can now run: remind");
        } else {
            char msg[MAX_PATH + 20];
            snprintf(msg, sizeof(msg), "You can run: %s", target_binary);
            print_log(LOG_INFO, msg);
        }
    } else {
        print_log(LOG_ERROR, "Installation verification failed - binary not found or not executable");
        return 1;
    }
    
    // Show man page info
    printf("\n");
    char man_msg[MAX_PATH + 50];
    snprintf(man_msg, sizeof(man_msg), "Manual page installed to: %s", target_man);
    print_log(LOG_INFO, man_msg);
    print_log(LOG_INFO, "You can view it with: man remind");
    
    printf("\n");
    print_log(LOG_SUCCESS, "Installation complete!");
    
    if (!system_install && !in_path(bin_dir)) {
        printf("\n");
        print_log(LOG_INFO, "Quick start options:");
        printf("  • Restart your terminal and run: remind\n");
        printf("  • Or run directly: ~/.local/bin/remind\n");
        printf("  • Or add ~/.local/bin to your PATH as shown above\n");
    }
    
    return 0;
}