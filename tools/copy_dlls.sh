#!/bin/bash
# Copy required DLLs from MSYS2/mingw to the deploy directory.
# Uses objdump to find DLL dependencies (works across architectures).
# Usage: copy_dlls.sh <exe_path> <dest_dir>

EXE="$1"
DEST="$2"

if [ ! -f "$EXE" ]; then
    echo "Warning: $EXE not found, skipping DLL copy"
    exit 0
fi

mkdir -p "$DEST"

# Find the mingw bin directory from the compiler in PATH
MINGW_BIN=$(dirname "$(which gcc)")
SEEN=""

copy_deps() {
    local binary="$1"
    for dllname in $(objdump -p "$binary" 2>/dev/null | grep "DLL Name:" | awk '{print $3}'); do
        # Skip if already processed
        case " $SEEN " in
            *" $dllname "*) continue ;;
        esac
        SEEN="$SEEN $dllname"

        local src="$MINGW_BIN/$dllname"
        if [ -f "$src" ]; then
            if [ ! -f "$DEST/$dllname" ] || [ "$src" -nt "$DEST/$dllname" ]; then
                cp "$src" "$DEST/"
                echo "  Copied: $dllname"
            fi
            # Recursively copy transitive dependencies
            copy_deps "$src"
        fi
    done
}

copy_deps "$EXE"
