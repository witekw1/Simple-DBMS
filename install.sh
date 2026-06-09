#!/usr/bin/env bash

set -e
CURR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$CURR_DIR/build"
cd "$CURR_DIR"


cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build
sudo cmake --install build
echo "**********************"
echo "Installed successfully"
