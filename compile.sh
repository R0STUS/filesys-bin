#!/usr/bin/env sh

# Move to the app directory
cd "$(dirname "$(readlink -f "$0")")" || exit

OUTPUT="fs-bin"
INPUT="src/main.cpp"
CONFIG="--std=c++20"

# Compile the app
exec g++ -o $OUTPUT $INPUT $CONFIG"$@"