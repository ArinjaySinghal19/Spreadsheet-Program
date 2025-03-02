# Compiler settings
CC = gcc
CFLAGS = -Wall -g

# Directories
BUILD_DIR = target/release
SRC = main.c
HEADERS = $(wildcard *.h)
EXECUTABLE = spreadsheet

# Main executable path
EXEC = $(BUILD_DIR)/$(EXECUTABLE)

# Test files
TEST_SRC = testsuite.c
TEST_EXEC = $(BUILD_DIR)/testsuite

# Default target
all: $(EXEC)

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the executable in release directory
$(EXEC): $(SRC) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRC) -lm

# Build the test suite
$(TEST_EXEC): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) -lm
	chmod +x testing.sh

# Target to build and run tests
test: $(EXEC) $(TEST_EXEC)
	$(TEST_EXEC)

# Generate report (placeholder for LaTeX compilation)
report:
	# Placeholder for LaTeX compilation
	echo "Report generation placeholder"

# Clean generated files
clean:
	rm -rf $(BUILD_DIR)
	rm -f output.txt

.PHONY: all clean test report
