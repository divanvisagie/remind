#!/bin/bash

# Script to generate README.md from remind.1 man page using pandoc
# Outputs exactly what's in the man page, nothing more, nothing less

set -e

MAN_PAGE="remind.1"
README="README.md"

# Check if man page exists
if [ ! -f "$MAN_PAGE" ]; then
    echo "Error: $MAN_PAGE not found"
    exit 1
fi

# Check if pandoc is available
if ! command -v pandoc >/dev/null 2>&1; then
    echo "Error: pandoc not found. Please install pandoc."
    exit 1
fi

# Convert man page via HTML intermediate format and clean up
echo "Generating README.md from $MAN_PAGE via HTML..."
pandoc -f man -t html "$MAN_PAGE" | \
pandoc -f html -t markdown | \
sed '/```{=html}/,/```/d' | \
sed 's/\\\$/$/g' > "$README"

echo "README.md generated successfully from $MAN_PAGE"
