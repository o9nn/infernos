#!/bin/bash
# Comprehensive test runner for all OpenCog Kernel AGI tests
# Runs C unit tests and Python tests

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Counters
TOTAL_SUITES=0
PASSED_SUITES=0
FAILED_SUITES=0

log() { echo -e "${GREEN}[TEST] $1${NC}"; }
warn() { echo -e "${YELLOW}[WARN] $1${NC}"; }
error() { echo -e "${RED}[ERROR] $1${NC}"; }
info() { echo -e "${BLUE}[INFO] $1${NC}"; }

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

run_test_suite() {
    local name="$1"
    local cmd="$2"

    TOTAL_SUITES=$((TOTAL_SUITES + 1))
    log "Running: $name"

    if eval "$cmd"; then
        log "PASSED: $name"
        PASSED_SUITES=$((PASSED_SUITES + 1))
        return 0
    else
        error "FAILED: $name"
        FAILED_SUITES=$((FAILED_SUITES + 1))
        return 1
    fi
}

echo ""
echo "========================================"
echo "OpenCog Kernel AGI - Complete Test Suite"
echo "========================================"
echo ""

# Phase 1: Build C Tests
info "Phase 1: Building C Unit Tests"
cd "$SCRIPT_DIR"

if make all; then
    log "C tests built successfully"
else
    error "Failed to build C tests"
    exit 1
fi

echo ""

# Phase 2: Run C Unit Tests
info "Phase 2: Running C Unit Tests"
echo ""

run_test_suite "OpenCog Kernel Tests" "./test_opencog_kernel" || true
echo ""
run_test_suite "Device Interface Tests" "./test_devopencog" || true
echo ""
run_test_suite "Process Cognitive Tests" "./test_process_cognitive" || true
echo ""
run_test_suite "lib9 Function Tests" "./test_lib9_functions" || true
echo ""
run_test_suite "Integration/Stress Tests" "./test_integration_stress" || true
echo ""

# Phase 3: Run Python Tests
info "Phase 3: Running Python Tests"
cd "$PROJECT_ROOT"

if command -v python3 &> /dev/null; then
    if [ -d "python/tests" ]; then
        run_test_suite "Python Cognitive Grammar Tests" \
            "cd python && python3 -m pytest tests/test_cognitive_grammar.py -v 2>/dev/null || python3 tests/test_cognitive_grammar.py" || true
        echo ""
        run_test_suite "Python Distributed Network Tests" \
            "cd python && python3 -m pytest tests/test_distributed_network.py -v 2>/dev/null || python3 tests/test_distributed_network.py" || true
        echo ""
        run_test_suite "Python Cognitive Network Tests" \
            "cd python && python3 -m pytest tests/test_cognitive_network.py -v 2>/dev/null || python3 tests/test_cognitive_network.py" || true
    else
        warn "Python test directory not found, skipping Python tests"
    fi
else
    warn "Python3 not available, skipping Python tests"
fi

echo ""

# Phase 4: Integration Test (existing)
info "Phase 4: Running Legacy Integration Test"
cd "$PROJECT_ROOT"

if [ -f "opencog_integration_test.c" ]; then
    if gcc -Wall -o /tmp/opencog_integration_test opencog_integration_test.c -lm 2>/dev/null; then
        run_test_suite "OpenCog Integration Test" "/tmp/opencog_integration_test" || true
        rm -f /tmp/opencog_integration_test
    else
        warn "Could not compile legacy integration test"
    fi
fi

echo ""

# Summary
echo "========================================"
echo "TEST SUITE SUMMARY"
echo "========================================"
echo ""
log "Total test suites: $TOTAL_SUITES"
log "Passed: $PASSED_SUITES"

if [ $FAILED_SUITES -eq 0 ]; then
    log "Failed: $FAILED_SUITES"
    echo ""
    echo -e "${GREEN}ALL TEST SUITES PASSED!${NC}"
    exit 0
else
    error "Failed: $FAILED_SUITES"
    echo ""
    echo -e "${RED}SOME TEST SUITES FAILED!${NC}"
    exit 1
fi
