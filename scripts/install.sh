#!/bin/bash

# Smart installation script for remind
# Detects user privileges and installs to appropriate location

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Check if we're in the project root
if [[ ! -f "src/main.c" || ! -f "remind.1" ]]; then
    print_error "This script must be run from the remind project root directory"
    exit 1
fi

# Build the project first
print_info "Building remind..."
if ! make main; then
    print_error "Failed to build remind"
    exit 1
fi

# Check if binary exists
if [[ ! -f "bin/remind" ]]; then
    print_error "Binary not found at bin/remind"
    exit 1
fi

# Detect if we're running as root or have sudo privileges
is_root=false
has_sudo=false

if [[ $EUID -eq 0 ]]; then
    is_root=true
    print_info "Running as root"
elif sudo -n true 2>/dev/null; then
    has_sudo=true
    print_info "User has sudo privileges"
else
    print_info "Running as regular user"
fi

# Determine installation paths
if [[ "$is_root" == true ]] || [[ "$has_sudo" == true ]]; then
    # System-wide installation
    BIN_DIR="/usr/local/bin"
    MAN_DIR="/usr/local/share/man/man1"
    INSTALL_TYPE="system-wide"

    # Ask user preference
    echo
    print_info "You have administrative privileges. Choose installation type:"
    echo "  1) System-wide installation (/usr/local/bin) - recommended"
    echo "  2) User-local installation (~/.local/bin)"
    echo
    if [[ -t 0 ]]; then
        read -p "Enter choice [1]: " choice
    else
        choice="1"
        echo "Enter choice [1]: $choice"
    fi
    choice=${choice:-1}

    if [[ "$choice" == "2" ]]; then
        BIN_DIR="$HOME/.local/bin"
        MAN_DIR="$HOME/.local/share/man/man1"
        INSTALL_TYPE="user-local"
        is_root=false
        has_sudo=false
    fi
else
    # User-local installation
    BIN_DIR="$HOME/.local/bin"
    MAN_DIR="$HOME/.local/share/man/man1"
    INSTALL_TYPE="user-local"
fi

print_info "Installing remind ($INSTALL_TYPE) to: $BIN_DIR"

# Create directories and install
if [[ "$is_root" == true ]]; then
    # Running as root
    mkdir -p "$BIN_DIR" "$MAN_DIR"
    cp bin/remind "$BIN_DIR/"
    cp remind.1 "$MAN_DIR/"
    chmod +x "$BIN_DIR/remind"
elif [[ "$has_sudo" == true ]]; then
    # Using sudo
    sudo mkdir -p "$BIN_DIR" "$MAN_DIR"
    sudo cp bin/remind "$BIN_DIR/"
    sudo cp remind.1 "$MAN_DIR/"
    sudo chmod +x "$BIN_DIR/remind"
else
    # User installation
    mkdir -p "$BIN_DIR" "$MAN_DIR"
    cp bin/remind "$BIN_DIR/"
    cp remind.1 "$MAN_DIR/"
    chmod +x "$BIN_DIR/remind"
fi

print_success "Installation completed!"

# Check and configure PATH for user installations
if [[ "$INSTALL_TYPE" == "user-local" ]]; then
    print_info "Checking PATH configuration..."

    # Check if ~/.local/bin is in PATH
    if [[ ":$PATH:" == *":$HOME/.local/bin:"* ]]; then
        print_success "~/.local/bin is already in your PATH"
    else
        print_warning "~/.local/bin is not in your PATH"

        # Detect shell and configuration file
        SHELL_NAME=$(basename "$SHELL")
        case "$SHELL_NAME" in
            bash)
                CONFIG_FILE="$HOME/.bashrc"
                ;;
            zsh)
                CONFIG_FILE="$HOME/.zshrc"
                ;;
            fish)
                CONFIG_FILE="$HOME/.config/fish/config.fish"
                ;;
            *)
                CONFIG_FILE="$HOME/.profile"
                ;;
        esac

        echo
        print_info "To add ~/.local/bin to your PATH, you can:"
        echo "  1) Run this command now (recommended):"
        if [[ "$SHELL_NAME" == "fish" ]]; then
            echo "     fish_add_path ~/.local/bin"
        else
            echo "     echo 'export PATH=\"\$HOME/.local/bin:\$PATH\"' >> $CONFIG_FILE"
        fi
        echo "  2) Add it manually to your shell configuration"
        echo "  3) Run 'remind' with full path: ~/.local/bin/remind"
        echo

        if [[ -t 0 ]]; then
            read -p "Would you like to add ~/.local/bin to your PATH now? [y/N]: " add_path
        else
            add_path="N"
            echo "Would you like to add ~/.local/bin to your PATH now? [y/N]: $add_path"
        fi
        if [[ "$add_path" =~ ^[Yy]$ ]]; then
            if [[ "$SHELL_NAME" == "fish" ]]; then
                if command -v fish_add_path >/dev/null 2>&1; then
                    fish_add_path "$HOME/.local/bin"
                    print_success "Added ~/.local/bin to PATH (fish)"
                else
                    echo "set -gx PATH \$HOME/.local/bin \$PATH" >> "$HOME/.config/fish/config.fish"
                    print_success "Added ~/.local/bin to PATH in $HOME/.config/fish/config.fish"
                fi
            else
                echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$CONFIG_FILE"
                print_success "Added ~/.local/bin to PATH in $CONFIG_FILE"
            fi

            print_info "Please restart your terminal or run: source $CONFIG_FILE"
        fi
    fi
fi

# Test the installation
echo
print_info "Testing installation..."

# Simply check if the binary exists and is executable
if [[ -x "$BIN_DIR/remind" ]]; then
    print_success "remind is installed at $BIN_DIR/remind"

    # Check if it's in PATH for system-wide installations
    if [[ "$INSTALL_TYPE" == "system-wide" ]]; then
        if command -v remind >/dev/null 2>&1; then
            print_success "remind is available in PATH"
            print_info "You can now run: remind"
        else
            print_warning "remind binary exists but may not be in PATH"
            print_info "Try running: hash -r && remind"
        fi
    elif [[ ":$PATH:" == *":$HOME/.local/bin:"* ]]; then
        print_success "remind should be available in PATH"
        print_info "You can now run: remind"
    else
        print_info "You can run: $BIN_DIR/remind"
    fi
else
    print_error "Installation verification failed - binary not found or not executable"
    exit 1
fi

# Show man page info
echo
print_info "Manual page installed to: $MAN_DIR/remind.1"
print_info "You can view it with: man remind"

echo
print_success "Installation complete!"

if [[ "$INSTALL_TYPE" == "user-local" ]] && [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
    echo
    print_info "Quick start options:"
    echo "  • Restart your terminal and run: remind"
    echo "  • Or run directly: ~/.local/bin/remind"
    echo "  • Or add ~/.local/bin to your PATH as shown above"
fi
