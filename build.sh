!#/bin/sh

set -xe

BIN=bin

if [ -d "$BIN" ]; then
  rm -rf "$BIN"
fi

mkdir -p "$BIN"

gcc -Wall -Wextra -ggdb -o ${BIN}/main main.c
