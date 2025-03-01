#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* Type definitions */
typedef int16_t short_int;

/**
 * Node structure for dependency tracking
 */
typedef struct Node {
    struct Node *next;
    short_int row;
    short_int col;
} Node;

/**
 * ParsedInput structure for representing cell formulas
 */
typedef struct {
    short_int target[2];         // Target cell (row, col)
    char expression_type;  // 0:value, 1:expression, 2:function, 3:sleep, 4:no assignment
    char operator;         // Operator (+, -, *, /)
    bool is_value_1;             // If cell 1 is a number or cell reference
    bool is_value_2;             // If cell 2 is a number or cell reference
    union {
        // Value data (expression_type = 0)
        struct {
            int value;
        } value_data;

        // Expression data (expression_type = 1)
        struct {
            int expression_cell[2];      // Cells in expression
        } expression_data;

        // Function data (expression_type = 2)
        struct {
            int function_range[2];       // Function range
        } function_data;

        // Sleep data (expression_type = 3)
        struct {
            int sleep_value;
        } sleep_data;
    } content;
} ParsedInput;



/**
 * Cell structure representing a spreadsheet cell
 */
typedef struct cell {
    ParsedInput parsed;
    Node *dependencies;
    int value;         
    bool is_dirty;
    bool is_in_stack;
    
} cell;

/**
 * Initialize a ParsedInput structure with default values
 */
void initialize_parsed_input(ParsedInput* input) {
    if (input == NULL) return;
    
    // Initialize target coordinates
    input->target[0] = 0;
    input->target[1] = 0;
    
    // Initialize all union members to 0
    memset(&input->content, 0, sizeof(input->content));
    
    // Initialize expression type
    input->expression_type = '4';
}

/**
 * Check if dimensions for the spreadsheet are valid
 */
bool valid_input(short_int rows, short_int cols) {
    if (rows < 1 || rows > 999 || cols < 1 || cols > 18278) {
        return false;
    }
    return true;
}

/**
 * Initialize the spreadsheet with empty cells
 */
void initialize_sheet(cell ***sheet, short_int rows, short_int cols) {
    // Allocate array of pointers to rows
    *sheet = (cell **)malloc(rows * sizeof(cell *));
    if (*sheet == NULL) {
        return;
    }

    // Allocate array of cells for each row
    for (short_int i = 0; i < rows; i++) {
        (*sheet)[i] = (cell *)malloc(cols * sizeof(cell));
        if ((*sheet)[i] == NULL) {
            // Free previously allocated memory if allocation fails
            for (short_int j = 0; j < i; j++) {
                free((*sheet)[j]);
            }
            free(*sheet);
            return;
        }
    }

    // Initialize each cell
    for (short_int i = 0; i < rows; i++) {
        for (short_int j = 0; j < cols; j++) {
            (*sheet)[i][j].value = 0;
            (*sheet)[i][j].dependencies = NULL;
            (*sheet)[i][j].is_dirty = false;
            (*sheet)[i][j].is_in_stack = false;
            initialize_parsed_input(&(*sheet)[i][j].parsed);
        }
    }
}

/**
 * Free the linked list of dependencies
 */
void free_dependencies(Node *dependencies) {
    Node *temp = dependencies;
    while (temp != NULL) {
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
}

/**
 * Free all memory allocated for the spreadsheet
 */
void free_sheet(cell **sheet, short_int row, short_int col) {
    for (short_int i = 0; i < row; i++) {
        for (short_int j = 0; j < col; j++) {
            free_dependencies(sheet[i][j].dependencies);
        }
        free(sheet[i]);
    }
    free(sheet);
}

/**
 * Return the minimum of two values
 */
short_int min(short_int a, short_int b) {
    return a < b ? a : b;
}

/**
 * Return the maximum of two values
 */
short_int max(short_int a, short_int b) {
    return a > b ? a : b;
}

/**
 * Check if a string is a valid cell reference (e.g., "A1", "AA10")
 */
short_int is_valid_cell(const char *cell) {
    short_int i = 0;

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

/**
 * Parse a cell reference (e.g., "AA10" -> row=9, col=26)
 */
short_int parse_cell(const char *cell, int *value, short_int sheet_rows, short_int sheet_cols) {
    if (!is_valid_cell(cell)) return 0;

    short_int i = 0;
    while (isalpha(cell[i])) i++;

    if (i == strlen(cell)) return 0; // Invalid cell reference

    char col_part[16], row_part[16];
    strncpy(col_part, cell, i);
    col_part[i] = '\0';
    strcpy(row_part, cell + i);

    // Convert column part to index (e.g., "A"=0, "AA"=26)
    short_int col = 0, row = 0;
    for (short_int j = 0; col_part[j] != '\0'; j++) {
        col = col * 26 + (toupper(col_part[j]) - 'A' + 1);
    }
    
    if (col == 0) return 0; // Invalid column part
    col -= 1;
    
    if (row_part[0] == '\0') return 0; // Invalid row part
    
    for (short_int i = 1; row_part[i] != '\0'; i++) {
        if (!isdigit(row_part[i])) {
            return 0;
        }
    }
    
    row = atoi(row_part) - 1; // Convert to 0-based indexing
    
    if (row < 0 || row >= sheet_rows || col < 0 || col >= sheet_cols) return 0; // Out of bounds
    
    *value = (row << 16) | col;
    return 1;
}

/**
 * Get current time in seconds with microsecond precision
 */
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/**
 * Convert a string to a natural number, or return -1 if invalid
 */
int string_to_nat(char *s) {
    short_int cur = 0;
    short_int inval = 0;
    
    while (*s != '\0') {
        if (*s > '9' || *s < '0') {
            inval = 1;
            break;
        }
        cur *= 10;
        cur += (*s - '0');
        s++;
    }
    
    if (cur == 0) inval = 1;
    
    return inval ? -1 : cur;
}




// int main(){

//     printf("size of cell: %lu\n", sizeof(cell));
//     printf("size of parsed: %lu\n", sizeof(ParsedInput));
//     printf("size of node: %lu\n", sizeof(Node));
//     printf("size of Node*: %lu\n", sizeof(Node*));
//     printf("size of short_int: %lu\n", sizeof(short_int));
//     printf("size of test struct: %lu\n", sizeof(test));
//     printf("size of expression_data struct: %lu\n", sizeof(expression_data));
//     printf("size of test cell struct: %lu\n", sizeof(testcell));

    

// }