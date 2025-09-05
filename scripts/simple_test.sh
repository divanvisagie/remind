#!/bin/bash

# Simple functional test for remind application

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

BINARY="./bin/remind"
TESTS_PASSED=0
TESTS_FAILED=0

echo "Simple Functional Tests for Remind"
echo "=================================="

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: $BINARY not found. Run 'make' first.${NC}"
    exit 1
fi

# Setup test environment
TEST_HOME="/tmp/remind_simple_$$"
export HOME="$TEST_HOME"
REMIND_FILE="$HOME/.local/state/remind/reminders"

pass_test() {
    echo -e "${GREEN}✓ PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

fail_test() {
    echo -e "${RED}✗ FAIL: $1${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

# Test 1: Empty check creates file
echo "Test 1: Empty file behavior"
output=$($BINARY -c 2>&1)
if [ -z "$output" ] && [ -f "$REMIND_FILE" ] && [ ! -s "$REMIND_FILE" ]; then
    pass_test
else
    fail_test "Should create empty file and produce no output"
fi

# Test 2: Add first reminder
echo "Test 2: Add first reminder"
$BINARY -a "Buy groceries"
if [ -f "$REMIND_FILE" ] && grep -q "Buy groceries" "$REMIND_FILE"; then
    pass_test
else
    fail_test "Should add reminder to file"
fi

# Test 3: Add second reminder
echo "Test 3: Add second reminder"
$BINARY -a "Call mom"
line_count=$(wc -l < "$REMIND_FILE")
if [ "$line_count" -eq 2 ] && grep -q "Call mom" "$REMIND_FILE"; then
    pass_test
else
    fail_test "Should have 2 reminders in file"
fi

# Test 4: Display reminders
echo "Test 4: Display reminders"
output=$($BINARY -c)
if echo "$output" | grep -q "Reminders" && echo "$output" | grep -q "1. Buy groceries" && echo "$output" | grep -q "2. Call mom"; then
    pass_test
else
    fail_test "Should display numbered reminders with header"
fi

# Test 5: Delete reminder
echo "Test 5: Delete reminder"
$BINARY -d 1
line_count=$(wc -l < "$REMIND_FILE")
if [ "$line_count" -eq 1 ] && grep -q "Call mom" "$REMIND_FILE" && ! grep -q "Buy groceries" "$REMIND_FILE"; then
    pass_test
else
    fail_test "Should delete first reminder"
fi

# Test 6: Display after deletion
echo "Test 6: Display after deletion"
output=$($BINARY -c)
if echo "$output" | grep -q "1. Call mom" && ! echo "$output" | grep -q "Buy groceries"; then
    pass_test
else
    fail_test "Should show remaining reminder with correct numbering"
fi

# Cleanup
rm -rf "$TEST_HOME"

# Summary
echo
echo "Test Summary:"
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
