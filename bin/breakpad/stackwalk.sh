#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$THIS_DIR"/common.sh

if [ $# -lt 2 ]; then
  echo "Usage: $0 <path to DMP file> [path to symbols dir]"
  exit 1
fi

DMP_FILE="$1"
SYMS_PATH="$2"
if [ -z "$SYMS_PATH" ]; then
  SYMS_PATH=$(cat "$SYMS_FILE" 2>/dev/null)
fi
if [ ! -d "$SYMS_PATH" ]; then
  echo "No symbols path found"
  exit 1
fi

cmd="$THIS_DIR"/minidump_stackwalk
shift 2
"$cmd" "$@" "$DMP_FILE" "$SYMS_PATH"
