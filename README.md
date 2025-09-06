# remind

A simple command-line reminder manager inspired by the classic BSD `mail(1)` utility.

## Overview

`remind` is a lightweight tool for managing personal reminders stored as plain text. It follows the Unix philosophy of simple commands, plain text storage, and direct editing capabilities.

### Shell Integration Example

Add `remind -c` to your shell startup file to see reminders when you open a terminal:

```bash
# Add this to your ~/.bashrc, ~/.zshrc, etc.
remind -c
```

**Note**: Reminders only appear when you have items to show - if your list is empty, you see nothing. This ability to reach "inbox zero" encourages you to complete your reminders and treat seeing them in your terminal as something that should not be the norm. This is part of the core philosophy of `remind`.

When you have reminders, you'll see them displayed like this:

```sh
###########################################
#                 Reminders               #
###########################################
1. Buy groceries for dinner
2. Call dentist to schedule appointment
3. Review pull request #42

$ #
```

When your reminder list is empty, you see a clean terminal with no output:

```
$ # Opening a new terminal session with no reminders
$ # Clean! No clutter - just your normal prompt
```

This makes it impossible to forget important tasks - they're shown whenever you start working, but only when there's something to remember!

### Basic Workflow

Here's how simple it is to use `remind`:

```sh
# Add some reminders
$ remind -a "Buy groceries for dinner"
$ remind -a "Call dentist to schedule appointment"
$ remind -a "Review pull request #42"

# View your reminders
$ remind -c
###########################################
#                 Reminders               #
###########################################
1. Buy groceries for dinner
2. Call dentist to schedule appointment
3. Review pull request #42

# Complete a task and remove it
$ remind -d 1    # Removes "Buy groceries for dinner"

# Check again - task is gone
$ remind -c
###########################################
#                 Reminders               #
###########################################
1. Call dentist to schedule appointment
2. Review pull request #42
```

## Features

- **Simple command-line interface** with intuitive flags
- **Plain text storage** - reminders stored as one line per reminder
- **Direct editing** - open reminder file in your `$EDITOR`
- **Cross-platform** - works on Linux, macOS, and other Unix-like systems
- **No dependencies** - single binary with no external requirements

## Installation

### From Source

1. Clone the repository:
   ```sh
   git clone https://github.com/divanvisagie/remind.git
   cd remind
   ```

2. Build the binary:
   ```bash
   make
   ```

3. Install (optional):
   ```bash
   make install
   ```

### Quick Start

```sh
# Add a reminder
remind -a "Buy groceries"

# List all reminders
remind -c

# Delete a specific reminder (by line number)
remind -d 2

# Edit reminders manually
remind

# Get help
remind --help
```

## Usage

```
remind [OPTIONS]

OPTIONS:
    -c              Check reminders. Prints the current list of reminders.
    -a TEXT         Add a new reminder line containing TEXT.
    -d N            Delete reminder at line number N (1-based).
    -h, --help      Show help message.
    (no options)    Open the reminders file in $EDITOR for manual editing.
```

## Files

Reminders are stored in `$HOME/.local/state/remind/reminders` as plain text, one reminder per line.

## Integration

Add to your shell startup file (`.bashrc`, `.zshrc`, etc.) to see reminders when you open a terminal:

```bash
echo 'remind -c' >> ~/.zshrc
```

## Building

Requirements:
- GCC or compatible C compiler
- Make

```bash
make            # Build the binary
make test       # Run tests
make clean      # Clean build artifacts
make install    # Install to system
```

## Documentation

Full documentation is available in the `docs/` directory:

- **[Online Documentation](https://divanv.com/remind/)** - Browse the complete documentation
- **[Manual Page](https://divanv.com/remind/remind.1.html)** - Complete reference documentation (`man remind`)
- **Generate docs**: Run `make docs` to regenerate HTML documentation

You can also view the manual page directly:
```sh
man ./remind.1          # View local man page
make docs && open docs/index.html  # Generate and view docs
```

## Development

### Running Tests

```bash
make test       # Run functional tests
make test-all   # Run all tests including memory checks
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Ensure all tests pass (`make test-all`)
6. Submit a pull request

## License

This project is licensed under the BSD 3-Clause License. See [LICENSE.md](LICENSE.md) for details, or view the [online license](https://divanv.com/remind/LICENSE.html).

## Author

Divan Visagie, 2025

Inspired by the classic `mail(1)` utility from BSD Unix.
