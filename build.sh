#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'USAGE'
Usage: $0 [--static] [--output-dir DIR]
Update apt, install build dependencies, configure, build, and test es-shell.

Options:
  --static          Attempt to link es statically
  --output-dir DIR  Directory to place built binary (default: bin)
  -h, --help        Show this help message and exit
USAGE
}

static=0
out_dir="bin"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --static)
            static=1
            shift
            ;;
        --output-dir)
            out_dir="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac

done

set -x
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential libtool autoconf automake pkg-config bison flex
libtoolize -qi
autoreconf -i
if [[ $static -eq 1 ]]; then
    ./configure LDFLAGS="-static"
else
    ./configure
fi
make clean
make
make test
mkdir -p "$out_dir"
cp es "$out_dir/es-shell"
