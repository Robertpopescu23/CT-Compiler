#!/bin/bash
#
# run_tests.sh — Build the AtomC compiler and run it against all test/example files.
# Usage: ./run_tests.sh
#

CC=gcc
CFLAGS="-Wall -Wextra"
SRC="main.c LA.c SY.c DA.c"
BINARY="./atomc"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  AtomC Compiler — Test Suite Runner${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

# ─────────────────────────────────────────────
# Step 1: Compile the compiler
# ─────────────────────────────────────────────
echo -e "${YELLOW}[BUILD]${NC} Compiling the AtomC compiler..."
if $CC $CFLAGS -o "$BINARY" $SRC 2>&1; then
    echo -e "${GREEN}[BUILD]${NC} ✅ Compiler built successfully → ${BINARY}"
else
    echo -e "${RED}[BUILD]${NC} ❌ Compilation FAILED. Fix errors above."
    exit 1
fi
echo ""

# ─────────────────────────────────────────────
# Step 2: Define test files
# ─────────────────────────────────────────────
# Files that SHOULD parse successfully (valid programs)
VALID_FILES=(
    "test_lexer.c"
    "test_type_analysis.c"
)

# Files that SHOULD produce an error (contains deliberate mistakes)
# Files that SHOULD fail: deliberate errors, unsupported features, or undefined identifiers
ERROR_FILES=(
    "test_domain_analysis.c"    # syntax bug on line 76 (missing parentheses)
    "test_parser.c"             # uses i++ which lexer doesn't tokenize
    "example1.c"                # uses undefined external function 'puti'
    "example2.c"                # uses undefined external function 'puti'
    "aa.c"                      # multi-dimensional array (unsupported)
)

PASS=0
FAIL=0
EXPECTED_ERRORS=0

# ─────────────────────────────────────────────
# Step 3: Run tests on valid files
# ─────────────────────────────────────────────
echo -e "${YELLOW}[TEST]${NC} Running valid-program tests..."
echo "────────────────────────────────────────"

for file in "${VALID_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "  ${YELLOW}⚠${NC}  ${file} — SKIPPED (file not found)"
        continue
    fi

    output=$($BINARY "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo -e "  ${GREEN}✅${NC} ${file} — PASSED (exit code 0)"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}❌${NC} ${file} — FAILED (exit code ${exit_code})"
        if [ -n "$output" ]; then
            echo -e "      Output: ${output}"
        fi
        FAIL=$((FAIL + 1))
    fi
done
echo ""

# ─────────────────────────────────────────────
# Step 4: Run tests on files expected to error
# ─────────────────────────────────────────────
echo -e "${YELLOW}[TEST]${NC} Running expected-error tests..."
echo "────────────────────────────────────────"

for file in "${ERROR_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "  ${YELLOW}⚠${NC}  ${file} — SKIPPED (file not found)"
        continue
    fi

    output=$($BINARY "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -ne 0 ]; then
        echo -e "  ${GREEN}✅${NC} ${file} — Correctly produced error (exit code ${exit_code})"
        if [ -n "$output" ]; then
            echo -e "      Error: ${output}"
        fi
        EXPECTED_ERRORS=$((EXPECTED_ERRORS + 1))
    else
        echo -e "  ${RED}❌${NC} ${file} — Should have errored but PASSED (exit code 0)"
        FAIL=$((FAIL + 1))
    fi
done
echo ""

# ─────────────────────────────────────────────
# Step 5: Summary
# ─────────────────────────────────────────────
TOTAL=$((PASS + EXPECTED_ERRORS + FAIL))
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  Test Summary${NC}"
echo -e "${CYAN}========================================${NC}"
echo -e "  Valid tests passed:       ${GREEN}${PASS}${NC}"
echo -e "  Expected errors caught:   ${GREEN}${EXPECTED_ERRORS}${NC}"
echo -e "  Failures:                 ${RED}${FAIL}${NC}"
echo -e "  Total tests:              ${TOTAL}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}🎉 All tests behaved as expected!${NC}"
else
    echo -e "${RED}⚠  Some tests did not behave as expected. Review above.${NC}"
fi

# Cleanup the built binary
rm -f "$BINARY"
