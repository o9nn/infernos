# Makefile for OpenCog Kernel Unit Tests
# Builds and runs all C-based unit tests

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -std=c99
LDFLAGS = -lm

# Test executables
TESTS = test_opencog_kernel \
        test_devopencog \
        test_process_cognitive \
        test_lib9_functions \
        test_integration_stress \
        test_tensor_logic

# Build all tests
all: $(TESTS)

# Individual test targets
test_opencog_kernel: test_opencog_kernel.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_devopencog: test_devopencog.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_process_cognitive: test_process_cognitive.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_lib9_functions: test_lib9_functions.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_integration_stress: test_integration_stress.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_tensor_logic: test_tensor_logic.c
	$(CC) $(CFLAGS) -I../agentic_cognitive_grammar/tensor_logic -o $@ $< $(LDFLAGS)

# Run all tests
run: all
	@echo ""
	@echo "========================================"
	@echo "Running All Unit Tests"
	@echo "========================================"
	@echo ""
	@failed=0; \
	for test in $(TESTS); do \
		echo ">>> Running $$test..."; \
		./$$test || failed=$$((failed + 1)); \
		echo ""; \
	done; \
	echo "========================================"; \
	if [ $$failed -eq 0 ]; then \
		echo "ALL TEST SUITES PASSED!"; \
	else \
		echo "$$failed TEST SUITE(S) FAILED!"; \
		exit 1; \
	fi

# Run individual test suites
run-kernel: test_opencog_kernel
	./test_opencog_kernel

run-device: test_devopencog
	./test_devopencog

run-process: test_process_cognitive
	./test_process_cognitive

run-lib9: test_lib9_functions
	./test_lib9_functions

run-integration: test_integration_stress
	./test_integration_stress

run-tensor: test_tensor_logic
	./test_tensor_logic

# Quick test (just build and run basic tests)
quick: test_opencog_kernel test_lib9_functions
	@echo "Running quick tests..."
	./test_opencog_kernel
	./test_lib9_functions

# Clean up
clean:
	rm -f $(TESTS)
	rm -f *.o

# Check for memory leaks with valgrind (if available)
valgrind: all
	@for test in $(TESTS); do \
		echo ">>> Valgrind: $$test"; \
		valgrind --leak-check=full --error-exitcode=1 ./$$test 2>&1 | tail -20; \
	done

# Help target
help:
	@echo "OpenCog Kernel Test Suite Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all tests"
	@echo "  run          - Build and run all tests"
	@echo "  run-kernel   - Run OpenCog kernel tests"
	@echo "  run-device   - Run device interface tests"
	@echo "  run-process  - Run process cognitive tests"
	@echo "  run-lib9     - Run lib9 utility tests"
	@echo "  run-integration - Run integration/stress tests"
	@echo "  run-tensor   - Run tensor logic tests"
	@echo "  quick        - Run quick test subset"
	@echo "  clean        - Remove built test executables"
	@echo "  valgrind     - Run tests with valgrind"
	@echo "  help         - Show this help message"

.PHONY: all run run-kernel run-device run-process run-lib9 run-integration run-tensor quick clean valgrind help
