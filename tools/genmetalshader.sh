#!/bin/sh
set -o pipefail
xcrun -sdk macosx metal -c "$1" -o - |
xcrun -sdk macosx metallib -    -o - |
xxd -i > "$2"
