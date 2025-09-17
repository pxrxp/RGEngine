#!/bin/bash
set -e

# Save current dir, move to script dir
ORIGINAL_DIR="$(pwd)"
cd "$(dirname "${BASH_SOURCE[0]}")"

BINARY="./build/RivalGrounds"

if [ -f "$BINARY" ]; then
    echo "Running $BINARY..."
    "$BINARY"
else
    echo "Binary not found. Building first..."
    ./build.sh
    echo "Running $BINARY..."
    "$BINARY"
fi

cd "$ORIGINAL_DIR"
