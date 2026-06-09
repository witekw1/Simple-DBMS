#!/usr/bin/env bash
CURR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"

cmake -S . -B build
cmake --build build
sudo cmake --install build
echo "**********************"
echo "Installed successfully"
