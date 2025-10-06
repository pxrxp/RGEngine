#!/bin/bash
set -e

# Save current dir, move to script dir
ORIGINAL_DIR="$(pwd)"
cd "$(dirname "${BASH_SOURCE[0]}")"

DEBUGGER=""

while getopts "glh" opt; do
    case $opt in
        g)
            if [ -n "$DEBUGGER" ]; then
                echo "Error: -g and -l are mutually exclusive. Please choose only one." >&2
                exit 1
            fi
            DEBUGGER="gdb"
            ;;
        l)
            if [ -n "$DEBUGGER" ]; then
                echo "Error: -g and -l are mutually exclusive. Please choose only one." >&2
                exit 1
            fi
            DEBUGGER="lldb"
            ;;
        h)
            echo "Usage: $0 [-g | -l]"
            echo "  -g: Run with GDB"
            echo "  -l: Run with LLDB"
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
    esac
done


BINARY="./build/RivalGrounds"

if [ ! -f "$BINARY" ]; then
    echo "Binary not found. Building first..."
    ./build.sh
fi

if [ "$DEBUGGER" = "gdb" ]; then
    echo "Debugging $BINARY using GDB..."
    gdb $BINARY
elif [ "$DEBUGGER" = "lldb" ]; then
    echo "Debugging $BINARY using LLDB..."
    lldb $BINARY
else
    echo "Running $BINARY..."
    $BINARY
fi

cd "$ORIGINAL_DIR"
