#!/usr/bin/env bash

source "$(cd "$(dirname "${BASH_SOURCE[0]}")"&&pwd)"/common.sh

cmd="$THIS_DIR"/make_symbols.sh

bash "$cmd" "$1" "$2" Qt6Cored.pdb
bash "$cmd" "$1" "$2" Qt6Widgetsd.pdb
bash "$cmd" "$1" "$2" Qt6Guid.pdb
#todo: add more