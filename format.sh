#!/bin/sh
# format all C source and header files with clang-format
set -e

clang-format -i $(git ls-files '*.c' '*.h')
