#!/bin/sh

set -xe

BIN=bin
CC=clang

if [ -d "$BIN" ]; then
  rm -rf "$BIN"
fi

mkdir -p "$BIN"

${CC} -Wall -Wextra -ggdb -o ${BIN}/main main.c
