#!/usr/bin/env bash
# Remove generated build files and compiled binaries, but keep tracked
# placeholder READMEs (bin/README.md).
# Usage: ./clean.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

rm -rf -- "$SCRIPT_DIR/build"
mkdir -p -- "$SCRIPT_DIR/build"
if [ -d "$SCRIPT_DIR/bin" ]; then
	find "$SCRIPT_DIR/bin" -mindepth 1 -maxdepth 1 ! -name 'README.md' -exec rm -rf -- {} +
else
	mkdir -p -- "$SCRIPT_DIR/bin"
fi

# Keep tracked placeholder READMEs if they exist; recreate dir only.
echo "Cleaned build/ and bin/."
