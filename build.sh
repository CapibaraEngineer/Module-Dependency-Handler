#!/usr/bin/env bash
# Generate project files with Premake and build with Ninja (default) or make.
# Usage:
#   ./build.sh [Debug|Release|Sanitize] [--cc=clang|gcc] [--gmake] [--verbose]
#   ./build.sh --help
set -euo pipefail

CONFIG="Debug"
CC_OPT="--cc=clang"
GENERATOR="ninja"
VERBOSE=0

for arg in "$@"; do
	case "$arg" in
		Debug|Release|Sanitize) CONFIG="$arg" ;;
		--cc=*) CC_OPT="$arg" ;;
		--gmake) GENERATOR="gmake" ;;
		--verbose) VERBOSE=1 ;;
		-h|--help)
			sed -n '2,5p' "$0"
			exit 0
			;;
		*) echo "Unknown argument: $arg" >&2; exit 1 ;;
	esac
done

command -v premake5 >/dev/null || {
	echo "Error: premake5 not found. See premake/README.md." >&2
	exit 1
}

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PREMAKE_DIR="$SCRIPT_DIR/premake"
PREMAKE_FILE="$PREMAKE_DIR/premake5.lua"

if [ "$GENERATOR" = "ninja" ]; then
	command -v ninja >/dev/null || {
		echo "Error: ninja not found. Install ninja or use ./build.sh --gmake." >&2
		exit 1
	}
fi

echo "==> premake5 $GENERATOR ($CC_OPT, config=$CONFIG)"
# shellcheck disable=SC2086
premake5 --file="$PREMAKE_FILE" $CC_OPT $GENERATOR

# GCC writes compiled modules (*.gcm) to gcm.cache/ in the build dir and
# does not create it itself; Clang manages its own module cache.
mkdir -p "$SCRIPT_DIR/build/gcm.cache"

if [ "$GENERATOR" = "ninja" ]; then
	# The ninja backend exposes one phony target per project+config.
	mapfile -t TARGETS < <(sed -n 's/^[[:space:]]*project[[:space:]]*("\([^"]*\)").*/\1_'"$CONFIG"'/p' "$PREMAKE_FILE")
	[ "${#TARGETS[@]}" -gt 0 ] || {
		echo "Error: no project() blocks found in $PREMAKE_FILE." >&2
		exit 1
	}
	echo "==> ninja ${TARGETS[*]}"
	if [ "$VERBOSE" -eq 1 ]; then
		ninja -C "$SCRIPT_DIR/build" -v "${TARGETS[@]}"
	else
		ninja -C "$SCRIPT_DIR/build" "${TARGETS[@]}"
	fi
else
	echo "==> make config=${CONFIG,,}"
	MAKE_ARGS=("config=${CONFIG,,}")
	[ "$VERBOSE" -eq 1 ] && MAKE_ARGS+=("verbose=1")
	make -C "$SCRIPT_DIR/build" "${MAKE_ARGS[@]}"
fi

echo "Binaries under bin/<Project>_<...>/<Config>/"
