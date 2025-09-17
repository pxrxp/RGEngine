#!/bin/bash
set -e

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
