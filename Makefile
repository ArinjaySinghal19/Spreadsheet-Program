# Compiler settings
CC = gcc
CFLAGS = -Wall -g

# Directories
BUILD_DIR = target/release
SRC = main.c
HEADERS = $(wildcard *.h)
EXEC = $(BUILD_DIR)/spreadsheet

# Test files
TEST_SRC = testsuite.c
TEST_EXEC = $(BUILD_DIR)/testsuite

# Ensure the build directory exists before compiling
all: $(EXEC)

# Build the executable
$(EXEC): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Build the test suite
$(TEST_EXEC): $(TEST_SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC)

# Target to build and run tests
test: $(EXEC) $(TEST_EXEC)
	./$(TEST_EXEC)

# Clean generated files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test
