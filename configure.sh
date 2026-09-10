#!/usr/bin/env bash
# Structuring script for this Premake workspace.
# 1. Renames the "ProjectName" template project to a real name
#    (also renames the "ProjectNameTests" project and all path references).
# 2. Updates workspace/startproject names in premake/premake5.lua.
# 3. Optionally creates additional projects just like the template
#    (each uses common_settings(rootDir) from premake/common.lua).
#
# Layout: <Name>/{include,modules,source}/, shared tests in top-level tests/.
# Usage: ./configure.sh   (all interactive, no arguments)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PREMAKE_FILE="$SCRIPT_DIR/premake/premake5.lua"
TEMPLATE="ProjectName"

die() {
	echo "Error: $*" >&2
	exit 1
}

valid_name() {
	# Letters, digits, '_', '-', '.'; must start with alnum or '_'.
	case "$1" in
		""|*[!A-Za-z0-9_.-]*|[^A-Za-z0-9_]*)
			return 1
			;;
	esac
	return 0
}

prompt_required() {
	# $1 = prompt text, $2 = default (optional). Echoes result.
	local prompt="$1" default="${2:-}" answer=""
	while true; do
		if [ -n "$default" ]; then
			read -rp "$prompt [$default]: " answer || echo
			[ -z "$answer" ] && answer="$default"
		else
			read -rp "$prompt: " answer || echo
		fi
		# Trim surrounding whitespace.
		answer="$(echo "$answer" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
		if [ -z "$answer" ]; then
			echo "Value cannot be empty."
			continue
		fi
		if ! valid_name "$answer"; then
			echo "Invalid name '$answer'. Use letters, digits, '_', '-', '.'."
			continue
		fi
		echo "$answer"
		return 0
	done
}

escape_sed_replacement() {
	# Escape &, \, and | (we use | as sed delimiter).
	printf '%s' "$1" | sed -e 's/[\\&|]/\\&/g'
}

project_exists_in_premake() {
	grep -Eq "^[[:space:]]*project[[:space:]]*\\(\"$1\"\\)" "$PREMAKE_FILE"
}

append_project_block() {
	# $1 = project name, $2 = kind (ConsoleApp|StaticLib|SharedLib)
	# Shared flags/target dirs come from common_settings() in premake/common.lua.
	local name="$1" kind="$2"
	cat >> "$PREMAKE_FILE" <<EOF

project("$name")
	location(rootDir .. "build/$name")
	kind("$kind")
	common_settings(rootDir)

	files({
		rootDir .. "$name/**.cpp",
		rootDir .. "$name/**.hpp",
		rootDir .. "$name/**.cppm",
	})

	includedirs({
		rootDir .. "$name/include",
		rootDir .. "$name/modules",
		rootDir .. "$name/source",
	})
EOF
}

[ -f "$PREMAKE_FILE" ] || die "premake file not found: $PREMAKE_FILE"

echo "== Workspace structuring =="

# --- 1. Rename template project ---
MAIN_NAME=""
if [ -d "$SCRIPT_DIR/$TEMPLATE" ]; then
	MAIN_NAME="$(prompt_required "Enter project name to replace \"$TEMPLATE\"")"
	if [ "$MAIN_NAME" != "$TEMPLATE" ]; then
		[ -e "$SCRIPT_DIR/$MAIN_NAME" ] && die "'$MAIN_NAME' already exists."
		mv -- "$SCRIPT_DIR/$TEMPLATE" "$SCRIPT_DIR/$MAIN_NAME"
		echo "Renamed $TEMPLATE/ -> $MAIN_NAME/"
	else
		echo "Keeping name '$TEMPLATE'."
	fi
else
	echo "Note: '$TEMPLATE/' not found, skipping rename."
	MAIN_NAME="$(prompt_required "Enter main project name" "$TEMPLATE")"
	if [ ! -d "$SCRIPT_DIR/$MAIN_NAME" ]; then
		echo "Warning: directory '$MAIN_NAME/' does not exist."
	fi
fi

# Point the generic template strings at the real main project.
if grep -q "$TEMPLATE" "$PREMAKE_FILE"; then
	esc_main="$(escape_sed_replacement "$MAIN_NAME")"
	sed -i "s|$TEMPLATE|$esc_main|g" "$PREMAKE_FILE"
	echo "Updated '$TEMPLATE' references in premake/premake5.lua -> '$MAIN_NAME'."
fi

# --- 2. Workspace + startup project ---
WORKSPACE_NAME="$(prompt_required "Enter workspace name" "$MAIN_NAME")"
START_PROJECT="$(prompt_required "Enter startup project (startproject)" "$MAIN_NAME")"

esc_ws="$(escape_sed_replacement "$WORKSPACE_NAME")"
esc_start="$(escape_sed_replacement "$START_PROJECT")"
sed -i -E "s|^[[:space:]]*workspace[[:space:]]*\(.*\)|workspace (\"$esc_ws\")|" "$PREMAKE_FILE"
sed -i -E "s|^[[:space:]]*startproject[[:space:]]*\(.*\)|    startproject (\"$esc_start\")|" "$PREMAKE_FILE"
echo "Set workspace \"$WORKSPACE_NAME\", startproject \"$START_PROJECT\"."

# --- 3. Create more projects (prompt loop) ---
while true; do
	answer=""
	read -rp "Create another project? [y/N]: " answer || echo
	case "$answer" in
		[Yy]|[Yy][Ee][Ss]) ;;
		*) break ;;
	esac

	NEW_NAME=""
	while true; do
		NEW_NAME="$(prompt_required "Enter new project name")"
		if [ -e "$SCRIPT_DIR/$NEW_NAME" ]; then
			echo "'$NEW_NAME' already exists in workspace root."
			continue
		fi
		if project_exists_in_premake "$NEW_NAME"; then
			echo "Project '$NEW_NAME' already defined in premake5.lua."
			continue
		fi
		break
	done

	echo "Select project kind:"
	echo "  1) ConsoleApp"
	echo "  2) StaticLib"
	echo "  3) SharedLib"
	KIND="ConsoleApp"
	while true; do
		choice=""
		read -rp "Enter choice [1-3] (default 1): " choice || echo
		[ -z "$choice" ] && choice="1"
		case "$choice" in
			1) KIND="ConsoleApp"; break ;;
			2) KIND="StaticLib"; break ;;
			3) KIND="SharedLib"; break ;;
			*) echo "Invalid choice. Pick 1, 2 or 3." ;;
		esac
	done

	mkdir -p -- "$SCRIPT_DIR/$NEW_NAME/include" "$SCRIPT_DIR/$NEW_NAME/modules" \
				"$SCRIPT_DIR/$NEW_NAME/source"
	append_project_block "$NEW_NAME" "$KIND"
	echo "Created $NEW_NAME/ ($KIND) and appended project(\"$NEW_NAME\") to premake5.lua."
done

# --- 4. Final check ---
if ! project_exists_in_premake "$START_PROJECT"; then
	echo "Warning: startproject \"$START_PROJECT\" has no matching project() block (yet)."
	echo "Defined projects:"
	grep -E "^[[:space:]]*project[[:space:]]*\\(" "$PREMAKE_FILE" || true
fi

echo "Done. Premake file: premake/premake5.lua"
echo "Next: ./cleanup_template.sh  (strip template READMEs/examples once you are happy with the layout)"
