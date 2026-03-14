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

# Clean old result files and temp files
rm -f tests/*.result tests/*.tmp

# Helper: read a binary .result file (two 32-bit LE ints) and check it
check_result() {
    local name="$1"
    local result_file="$2"

    if [ ! -f "$result_file" ]; then
        echo "CRASH (no result file)"
        CRASH=$((CRASH + 1))
        return
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
}

###############################################################################
# Part 1: IDE Smoke Test (--smoke flag)
###############################################################################

printf "%-30s" "smoke_test..."

# Run the IDE in smoke test mode with a companion PRG
if ! "$COMPILER" --smoke tests/test_smoke.prg > tests/smoke.log 2>&1; then
    # Non-zero exit = smoke test detected failures
    echo "FAIL (see tests/smoke.log)"
    FAIL=$((FAIL + 1))
else
    check_result "smoke_test" "tests/test_smoke.result"
fi

###############################################################################
# Part 2: Self-hosted language/runtime tests (compile + run each test_*.prg)
###############################################################################

for prg in tests/test_*.prg; do
    name=$(basename "$prg" .prg)

    # Skip the smoke companion PRG (already tested above via --smoke)
    if [ "$name" = "test_smoke" ]; then
        continue
    fi

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

    check_result "$name" "$result_file"
done

###############################################################################
# Summary
###############################################################################

echo ""
echo "Results: $PASS passed, $FAIL failed, $CRASH crashed"

if [ $FAIL -gt 0 ] || [ $CRASH -gt 0 ]; then
    exit 1
fi
exit 0
