#!/usr/bin/env bash
# Remove template scaffolding. Run ONCE, just after ./configure.sh:
#
#   1. Deletes placeholder READMEs (app dir, tests/, bin/).
#      premake/README.md (real docs) is kept.
#   2. Strips the template example code (example.hpp/.cpp/.cppm,
#      test_example.cpp), leaving a minimal main.cpp + tests/main.cpp stub,
#      and drops the shared-source references from the Tests project.
#   3. Resets the root README.md to just "# <WorkspaceName>".
#   4. Deletes LICENSE (the template's license, not the project's).
#   5. Self-deletes: removes configure.sh and this script.
#
# Usage: ./cleanup_template.sh [--yes]

set -euo pipefail

YES=0
for arg in "$@"; do
	case "$arg" in
		-y|--yes) YES=1 ;;
		-h|--help)
			sed -n '2,13p' "$0"
			exit 0
			;;
		*) echo "Unknown argument: $arg" >&2; exit 1 ;;
	esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PREMAKE_FILE="$SCRIPT_DIR/premake/premake5.lua"

die() {
	echo "Error: $*" >&2
	exit 1
}

[ -f "$PREMAKE_FILE" ] || die "premake file not found: $PREMAKE_FILE"

WORKSPACE="$(sed -n 's/^[[:space:]]*workspace[[:space:]]*("\([^"]*\)").*/\1/p' "$PREMAKE_FILE" | head -n 1)"
[ -n "$WORKSPACE" ] || die "could not parse workspace() from $PREMAKE_FILE"

APP="$(sed -n 's/^[[:space:]]*startproject[[:space:]]*("\([^"]*\)").*/\1/p' "$PREMAKE_FILE" | head -n 1)"
[ -n "$APP" ] || APP="$WORKSPACE"

echo "Workspace:  $WORKSPACE"
echo "Main app:   $APP"
echo "This will: delete placeholder READMEs, strip example sources,"
echo "           reset README.md, delete LICENSE,"
echo "           and delete configure.sh + this script."

if [ "$YES" -ne 1 ]; then
	answer=""
	read -rp "Continue? [y/N]: " answer || echo
	case "$answer" in
		[Yy]|[Yy][Ee][Ss]) ;;
		*) echo "Aborted."; exit 0 ;;
	esac
fi

# Generated binaries/objects from the template example: drop them first.
if [ -x "$SCRIPT_DIR/clean.sh" ]; then
	bash "$SCRIPT_DIR/clean.sh"
fi

# --- 1. Placeholder READMEs (premake/README.md is real docs: kept) ---
rm -f -- \
	"$SCRIPT_DIR/$APP/README.md" \
	"$SCRIPT_DIR/$APP/include/README.md" \
	"$SCRIPT_DIR/$APP/modules/README.md" \
	"$SCRIPT_DIR/$APP/source/README.md" \
	"$SCRIPT_DIR/tests/README.md" \
	"$SCRIPT_DIR/bin/README.md"
echo "Removed placeholder READMEs."

# --- 2. Example sources -> minimal stubs ---
rm -f -- \
	"$SCRIPT_DIR/$APP/include/example.hpp" \
	"$SCRIPT_DIR/$APP/source/example.cpp" \
	"$SCRIPT_DIR/$APP/modules/example.cppm" \
	"$SCRIPT_DIR/tests/test_example.cpp"

cat > "$SCRIPT_DIR/$APP/source/main.cpp" <<'EOF'
int main() {
	return 0;
}
EOF

cat > "$SCRIPT_DIR/tests/main.cpp" <<'EOF'
// Minimal test driver stub. Return non-zero on failure.
int main() {
	return 0;
}
EOF
echo "Stripped example code (minimal main.cpp + tests/main.cpp)."

# Tests project no longer compiles the (now deleted) shared example sources.
ESC_APP="$(printf '%s' "$APP" | sed -e 's/\./\\./g')"
sed -i "\|$ESC_APP/source/example\\.cpp|d; \|$ESC_APP/include/example\\.hpp|d; \|$ESC_APP/modules/example\\.cppm|d" "$PREMAKE_FILE"
sed -i "\|rootDir \\.\\. \"$ESC_APP/include\",|d; \|rootDir \\.\\. \"$ESC_APP/modules\",|d; \|rootDir \\.\\. \"$ESC_APP/source\",|d" "$PREMAKE_FILE"
sed -i "/^-- Separate tests binary\\./,/^-- external test framework required\\.$/c\\-- Separate tests binary. Everything under tests/." "$PREMAKE_FILE"
echo "Updated Tests project in premake/premake5.lua."

# --- 3. Root README: title only ---
printf '# %s\n' "$WORKSPACE" > "$SCRIPT_DIR/README.md"
echo "Reset README.md to '# $WORKSPACE'."

# --- 4. Template license is not the project's license ---
rm -f -- "$SCRIPT_DIR/LICENSE"
echo "Deleted LICENSE (add your own)."

# --- 5. Self-delete: template setup is done ---
rm -f -- "$SCRIPT_DIR/configure.sh" "$(readlink -f "$0")"
echo "Deleted configure.sh and cleanup_template.sh."
echo "Done. Run ./build.sh to verify the workspace builds."
