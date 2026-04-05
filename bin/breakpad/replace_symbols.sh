#!/usr/bin/env bash

if [ $# -lt 2 ]; then
  echo "Usage: $0 <path to symbols dir> <path to source sym file>"
  exit 1
fi

SYMBOLS_DIR=${1/%\//}
SOURCE_SYM=$2

if [ ! -f "$SOURCE_SYM" ]; then
  echo "Source sym file not found: $SOURCE_SYM"
  exit 1
fi

if [ ! -d "$SYMBOLS_DIR" ]; then
  mkdir -p "$SYMBOLS_DIR"
fi

seg=$(head -n 1 "$SOURCE_SYM" | cut -d' ' -f4-5)
id=$(echo "$seg" | cut -d' ' -f1)
name=$(echo "$seg" | cut -d' ' -f2)
mkdir -p "$SYMBOLS_DIR/$name/$id"
mv "$SOURCE_SYM" "$SYMBOLS_DIR/$name/$id/"