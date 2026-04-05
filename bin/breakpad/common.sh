#!/usr/bin/env bash


LINUX=1
WINDOWS=1
DARWIN=1

case "$(uname -s)" in
  LINUX)
    LINUX=0
    ;;
  DARWIN)
    DARWIN=0
    ;;
  CYGWIN*|MINGW32*|MSYS*|MINGW*)
    WINDOWS=0
    ;;
  *)
    echo "Unknown operating system"
    ;;
esac

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SYMS_FILE="$THIS_DIR/syms_path"