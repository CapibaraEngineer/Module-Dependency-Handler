#!/usr/bin/env bash
# Run the workspace startProject binary (build it first with ./build.sh).
# Usage:
#   ./run.sh [Debug|Release|Sanitize] [-- program args...]
#   ./run.sh --help
set -euo pipefail

CONFIG="Debug"
case "${1:-}" in
	Debug|Release|Sanitize) CONFIG="$1"; shift ;;
	-h|--help)
		sed -n '2,5p' "$0"
		exit 0
		;;
	"") ;;
	--) shift ;;
	*) echo "Unknown config: $1 (expected Debug|Release|Sanitize)" >&2; exit 1 ;;
esac

# Program arguments may follow an optional -- separator.
[ "${1:-}" = "--" ] && shift

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PREMAKE_FILE="$SCRIPT_DIR/premake/premake5.lua"

APP="$(sed -n 's/^[[:space:]]*startproject[[:space:]]*("\([^"]*\)").*/\1/p' "$PREMAKE_FILE" | head -n 1)"
[ -n "$APP" ] || {
	echo "Error: could not parse startproject() from $PREMAKE_FILE." >&2
	exit 1
}

BIN="$(find "$SCRIPT_DIR/bin/$APP" -type f -executable -name "$APP" 2>/dev/null | grep -i "/${CONFIG}/" | sort | head -n 1 || true)"
[ -n "$BIN" ] || {
	echo "Error: no $APP binary for $CONFIG under bin/. Run ./build.sh $CONFIG first." >&2
	exit 1
}

exec "$BIN" "$@"
