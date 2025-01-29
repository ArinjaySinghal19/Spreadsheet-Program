#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Define constants for program flow
#define EXIT_PROGRAM 1
#define CONTINUE_PROGRAM 0

// Structure to hold parsed input
typedef struct {
    int row;       // Row index (0-based)
    int col;       // Column index (0-based)
    char op;       // Operator ('=' for now, can be extended later)
    char value[64]; // Expression or value as a string
} ParsedInput;

// Check if a string is a valid cell reference (e.g., "A1", "AA10")
int is_valid_cell(const char *cell) {
    int i = 0;

    // Ensure the column part contains letters
    while (isalpha(cell[i])) i++;
    if (i == 0) return 0; // No letters present

    // Ensure the row part contains digits
    while (cell[i]) {
        if (!isdigit(cell[i])) return 0; // Invalid character in row part
        i++;
    }
    return 1; // Valid cell reference
}

// Parse a cell reference (e.g., "AA10" -> row=9, col=26)
int parse_cell(const char *cell, int *row, int *col) {
    if (!is_valid_cell(cell)) return 0;

    int i = 0;
    while (isalpha(cell[i])) i++;

    char col_part[16], row_part[16];
    strncpy(col_part, cell, i);
    col_part[i] = '\0';
    strcpy(row_part, cell + i);

    // Convert column part to index (e.g., "A"=0, "AA"=26)
    *col = 0;
    for (int j = 0; col_part[j] != '\0'; j++) {
        *col = *col * 26 + (toupper(col_part[j]) - 'A' + 1);
    }

    // Convert row part to index
    *row = atoi(row_part); // Convert to 0-based indexing

    return 1;
}

// Parse the input into a ParsedInput structure
int parse_input(const char *input, ParsedInput *parsed) {
    char cell[32], expr[128];

    // Extract cell reference and value/expression
    if (sscanf(input, "%31[^=]=%127s", cell, expr) != 2) {
        return 0; // Invalid format
    }

    // Parse the cell reference
    if (!parse_cell(cell, &parsed->row, &parsed->col)) {
        return 0; // Invalid cell reference
    }

    // Store the operator and value
    parsed->op = '=';
    strncpy(parsed->value, expr, sizeof(parsed->value) - 1);
    parsed->value[sizeof(parsed->value) - 1] = '\0';

    return 1; // Successfully parsed
}

// Simulate spreadsheet scrolling (simple placeholders)
void scroll_up() {
    printf("Scrolling up...\n");
}

void scroll_down() {
    printf("Scrolling down...\n");
}

void scroll_left() {
    printf("Scrolling left...\n");
}

void scroll_right() {
    printf("Scrolling right...\n");
}

// Handle user input
int handle_input(const char *input) {
    // Handle commands
    if (strcmp(input, "q") == 0) {
        return EXIT_PROGRAM; // Quit
    } else if (strcmp(input, "w") == 0) {
        scroll_up();
        return CONTINUE_PROGRAM;
    } else if (strcmp(input, "s") == 0) {
        scroll_down();
        return CONTINUE_PROGRAM;
    } else if (strcmp(input, "a") == 0) {
        scroll_left();
        return CONTINUE_PROGRAM;
    } else if (strcmp(input, "d") == 0) {
        scroll_right();
        return CONTINUE_PROGRAM;
    }

    // Handle formulas and assignments
    ParsedInput parsed;
    if (parse_input(input, &parsed)) {
        printf("Parsed Input:\n");
        printf("  Cell: (%d, %d)\n", parsed.row, parsed.col);
        printf("  Operator: %c\n", parsed.op);
        printf("  Value: %s\n", parsed.value);
        return CONTINUE_PROGRAM;
    }

    // If no valid input, show an error
    printf("Error: Unrecognized command\n");
    return CONTINUE_PROGRAM;
}

// Main function
int main() {
    char input[256];

    printf("Spreadsheet Program (Enter 'q' to quit):\n");
    while (1) {
        printf("> ");
        scanf("%s", input); // Read input from the user
        if (handle_input(input) == EXIT_PROGRAM) break; // Exit if "q" is entered
    }

    printf("Exiting program...\n");
    return 0;
}
