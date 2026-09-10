#!/usr/bin/env bash
# Build the workspace, then run the startProject binary.
# Usage:
#   ./build_run.sh [Debug|Release|Sanitize] [build options...] [-- program args...]
#   ./build_run.sh --help
#
# Example:
#   ./build_run.sh Release --cc=gcc -- --some-app-flag
set -euo pipefail

CONFIG="Debug"
BUILD_OPTS=()
PROG_ARGS=()
SEP=0
case "${1:-}" in
	Debug|Release|Sanitize) CONFIG="$1"; shift ;;
	-h|--help)
		sed -n '2,9p' "$0"
		exit 0
		;;
	"") ;;
	--) shift; SEP=1 ;;
	*) echo "Unknown config: $1 (expected Debug|Release|Sanitize)" >&2; exit 1 ;;
esac

# Split at -- : left side goes to build.sh, right side to the program.
for arg in "$@"; do
	if [ "$SEP" -eq 0 ] && [ "$arg" = "--" ]; then
		SEP=1
		continue
	fi
	if [ "$SEP" -eq 0 ]; then
		BUILD_OPTS+=("$arg")
	else
		PROG_ARGS+=("$arg")
	fi
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

"$SCRIPT_DIR/build.sh" "$CONFIG" "${BUILD_OPTS[@]}"
"$SCRIPT_DIR/run.sh" "$CONFIG" -- "${PROG_ARGS[@]}"
