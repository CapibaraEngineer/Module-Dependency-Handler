#!/usr/bin/env bash
# Build the workspace and run every *Tests binary.
# Usage:
#   ./test.sh [Debug|Release|Sanitize] [--cc=clang|gcc] [--gmake]
#   ./test.sh --help
set -euo pipefail

CONFIG="Debug"
case "${1:-}" in
	Debug|Release|Sanitize) CONFIG="$1"; shift ;;
	-h|--help)
		sed -n '2,5p' "$0"
		exit 0
		;;
	"") ;;
	*) echo "Unknown config: $1 (expected Debug|Release|Sanitize)" >&2; exit 1 ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

"$SCRIPT_DIR/build.sh" "$CONFIG" "$@"

mapfile -t TESTS < <(find "$SCRIPT_DIR/bin" -type f -executable -name '*Tests' | grep -i "/${CONFIG}/" | sort || true)

[ "${#TESTS[@]}" -gt 0 ] || {
	echo "No *Tests binaries found for $CONFIG under bin/." >&2
	exit 1
}

fail=0
for t in "${TESTS[@]}"; do
	echo "==> $t"
	if "$t"; then
		:
	else
		fail=1
	fi
done

exit "$fail"
