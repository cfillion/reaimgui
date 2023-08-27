#!/bin/sh
set -o pipefail
xcrun -sdk macosx metal -c "$1" -o - \
  ${MACOSX_DEPLOYMENT_TARGET:+-mmacosx-version-min=$MACOSX_DEPLOYMENT_TARGET} |
xcrun -sdk macosx metallib - -o - |
xxd -i > "$2"
