#!/bin/sh

set -xe

SAMPLES=./samples

if [ ! -d "$SAMPLES" ]; then
  echo "There is now samples directory to test against"
  exit
else 
  find "$SAMPLES" -type f -print0 | xargs -0 -I {} sh -c 'file {} && ./bin/main {} && echo ""'
fi
