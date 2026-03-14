#!/bin/bash
# DIV Games Studio — Self-hosted test runner
# Compiles each test_*.prg, runs it, reads the binary .result file.
# Must be run from the div/ runtime directory (parent of tests/).

set -euo pipefail

COMPILER="system/div-WINDOWS.exe"
RUNNER="system/divrun-WINDOWS.exe"
PASS=0
FAIL=0
CRASH=0

# Clean old result files
rm -f tests/*.result

for prg in tests/test_*.prg; do
    name=$(basename "$prg" .prg)
    result_file="tests/${name}.result"

    printf "%-30s" "$name..."

    # Compile
    if ! "$COMPILER" -c "$prg" 2>/dev/null; then
        echo "COMPILE_ERROR"
        CRASH=$((CRASH + 1))
        continue
    fi

    # Run
    if ! "$RUNNER" system/EXEC.EXE 2>/dev/null; then
        # Check if result file was written before crash
        if [ ! -f "$result_file" ]; then
            echo "CRASH"
            CRASH=$((CRASH + 1))
            continue
        fi
    fi

    # Read binary result (two 32-bit little-endian ints)
    if [ ! -f "$result_file" ]; then
        echo "CRASH (no result file)"
        CRASH=$((CRASH + 1))
        continue
    fi

    # Parse two 32-bit ints from binary file
    read -r test_passed test_failed <<< "$(od -An -td4 -w4 "$result_file" | head -2 | tr '\n' ' ')"

    if [ "$test_failed" -gt 0 ] 2>/dev/null; then
        echo "FAIL (passed=$test_passed failed=$test_failed)"
        FAIL=$((FAIL + 1))
    elif [ "$test_passed" -gt 0 ] 2>/dev/null; then
        echo "PASS ($test_passed assertions)"
        PASS=$((PASS + 1))
    else
        echo "CRASH (bad result data)"
        CRASH=$((CRASH + 1))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed, $CRASH crashed"

if [ $FAIL -gt 0 ] || [ $CRASH -gt 0 ]; then
    exit 1
fi
exit 0
