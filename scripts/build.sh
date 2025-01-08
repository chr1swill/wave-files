!#/bin/sh

set -xe

ROOT="scripts"
BIN="${ROOT}/bin"

if [ -d "$BIN" ]; then
  rm -rf "$BIN"
fi

mkdir -p "$BIN"

g++ -Wall -Wextra -ggdb -o ${BIN}/to_base_16 ${ROOT}/to_base_16.cpp
