#!/bin/bash

# Valgrind test script for remind application
# This script runs comprehensive memory leak detection tests

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Valgrind options
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"

# Binary path
BINARY="./bin/remind-debug"

# Test file path
TEST_FILE="/tmp/remind_test_$$"

echo -e "${YELLOW}Running Valgrind Tests for Remind${NC}"
echo "========================================"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: $BINARY not found. Run 'make debug' first.${NC}"
    exit 1
fi

# Setup test environment
export HOME="/tmp"
mkdir -p "$HOME/.local/state/remind"
REMIND_FILE="$HOME/.local/state/remind/reminders"

# Create a test file with some sample reminders
cat > "$REMIND_FILE" << EOF
Buy groceries
Call mom
Fix the leaky faucet
Finish project report
Schedule dentist appointment
EOF

echo -e "${YELLOW}Test 1: Check reminders (-c)${NC}"
if valgrind $VALGRIND_OPTS $BINARY -c > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS: No memory leaks in check operation${NC}"
else
    echo -e "${RED}✗ FAIL: Memory leaks detected in check operation${NC}"
    echo "Running with verbose output:"
    valgrind $VALGRIND_OPTS $BINARY -c
    exit 1
fi

echo -e "${YELLOW}Test 2: Add reminder (-a)${NC}"
if valgrind $VALGRIND_OPTS $BINARY -a "Test reminder from valgrind" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS: No memory leaks in add operation${NC}"
else
    echo -e "${RED}✗ FAIL: Memory leaks detected in add operation${NC}"
    echo "Running with verbose output:"
    valgrind $VALGRIND_OPTS $BINARY -a "Test reminder from valgrind"
    exit 1
fi

echo -e "${YELLOW}Test 3: Delete reminder (-d)${NC}"
if valgrind $VALGRIND_OPTS $BINARY -d 1 > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS: No memory leaks in delete operation${NC}"
else
    echo -e "${RED}✗ FAIL: Memory leaks detected in delete operation${NC}"
    echo "Running with verbose output:"
    valgrind $VALGRIND_OPTS $BINARY -d 1
    exit 1
fi

echo -e "${YELLOW}Test 4: Check empty file${NC}"
> "$REMIND_FILE"  # Empty the file
if valgrind $VALGRIND_OPTS $BINARY -c > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS: No memory leaks with empty file${NC}"
else
    echo -e "${RED}✗ FAIL: Memory leaks detected with empty file${NC}"
    echo "Running with verbose output:"
    valgrind $VALGRIND_OPTS $BINARY -c
    exit 1
fi

echo -e "${YELLOW}Test 5: Invalid delete operation${NC}"
if valgrind $VALGRIND_OPTS $BINARY -d 999 > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS: No memory leaks in invalid delete${NC}"
else
    echo -e "${RED}✗ FAIL: Memory leaks detected in invalid delete${NC}"
    echo "Running with verbose output:"
    valgrind $VALGRIND_OPTS $BINARY -d 999
    exit 1
fi

# Test with large file (stress test)
echo -e "${YELLOW}Test 6: Large file stress test${NC}"
for i in {1..500}; do
    echo "Reminder item number $i" >> "$REMIND_FILE"
done

if valgrind $VALGRIND_OPTS $BINARY -c > /dev/null 2>&1; then
    echo -e "${GREEN}✓ PASS: No memory leaks with large file (500 lines)${NC}"
else
    echo -e "${RED}✗ FAIL: Memory leaks detected with large file${NC}"
    echo "Running with verbose output:"
    valgrind $VALGRIND_OPTS $BINARY -c
    exit 1
fi

# Cleanup
rm -rf "$HOME/.local"

echo ""
echo -e "${GREEN}All valgrind tests passed!${NC}"
echo "No memory leaks detected in any operation."
