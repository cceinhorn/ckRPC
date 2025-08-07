#!/bin/bash

set -e

# Get the current directory
ROOT_DIR=$(pwd)

# Clear and recreate the build directory
rm -rf "$ROOT_DIR/build"
mkdir "$ROOT_DIR/build"
cd "$ROOT_DIR/build"

# Run cmake and make
cmake ..
make

# Return to the root directory and copy the include and lib directories
# cd "$ROOT_DIR"
# cp -r "$ROOT_DIR/src/include" "$ROOT_DIR/lib"