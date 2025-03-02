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

# LaTeX report settings
REPORT_SRC = report.tex
REPORT_PDF = report.pdf

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

# Generate PDF report from LaTeX source
report: $(REPORT_PDF)

$(REPORT_PDF): $(REPORT_SRC)
	pdflatex $(REPORT_SRC)
	pdflatex $(REPORT_SRC)  # Run twice for proper cross-references

# Clean generated files
clean:
	rm -rf $(BUILD_DIR)
	rm -f output.txt
	rm -f *.aux *.log *.out *.toc *.lof *.lot

# Very clean - also removes the PDF
distclean: clean
	rm -f $(REPORT_PDF)

.PHONY: all clean distclean test report