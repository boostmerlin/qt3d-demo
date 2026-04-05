#!/usr/bin/env bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$THIS_DIR"/common.sh

if [ $# -lt 3 ]; then
  echo "Usage: $0 <path to symbols dir> <path to binary> [DGB_FILES, eg. pdb]"
  exit 1
fi

BINARY_DIR=$2
SYMBOL_DIR=$(realpath "$1")/
EXT=""
dump_cmd="$THIS_DIR"/dump_syms
shift 2

echo "$SYMBOL_DIR" > "$SYMS_FILE"

get_sym() {
  echo "Generate symbol file for $1"
  local file_name dump_cmd target_dir target_file seg id name
  file_name=$(basename "$1")
  file_name="${file_name%.*}"
  dump_cmd="$2"
  target_dir="$3"
  target_file="$target_dir$file_name.sym"
  "$dump_cmd" "$1" > "$target_file"
  seg=$(head -n 1 "$target_file" | cut -d' ' -f4-5)
  id=$(echo "$seg" | cut -d' ' -f1)
  name=$(echo "$seg" | cut -d' ' -f2)
  pushd "$target_dir" || exit
  mkdir -p "$name/$id"
  mv "$file_name.sym" "$name/$id/$file_name.sym"
}
export -f get_sym
while [ "$1" != "" ]; do
  find "$BINARY_DIR" -maxdepth 1 -name "$1$EXT" -exec bash -c 'get_sym "$0" "$1" "$2"' {} "$dump_cmd" "$SYMBOL_DIR" \;
  shift
done

