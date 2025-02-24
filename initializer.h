#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

typedef int16_t short_int;

typedef struct Node{
    short_int row;
    short_int col;
    struct Node *next;
} Node;


typedef struct {
    short_int target[2]; // Target cell (row, col)
    short_int expression_type; // 0 for value, 1 for expression, 2 for function, 3 for sleep, -1 for no assignment
    union {
        struct {
            // short_int value[2]; // Value (if expression_type=0)
            int value;
            bool is_value; // If value is a number or a cell reference
        } value_data;

        struct {
            // short_int expression_cell_1[2]; // First cell in expression
            // short_int expression_cell_2[2]; // Second cell in expression
            int expression_cell[2]; // Cell in expression
            char expression_operator; // Operator in expression ( +, -, *, /)
            bool is_value_1; // If cell 1 is a number or a cell reference
            bool is_value_2; // If cell 2 is a number or a cell reference
        } expression_data;

        struct {
            char function_operator; 
            // short_int function_range[4]; // Function range (start row, start col, end row, end col)
            int function_range[2]; // Function range
        } function_data;

        struct {
            // short_int sleep_value[2]; // Sleep value (if expression_type=3)
            int sleep_value;
            bool is_value; // If value is a number or a cell reference
        } sleep_data;
    } content;
} ParsedInput;

typedef struct cell{
    int value;
    short_int row;
    short_int col;
    Node *dependencies;
    bool is_dirty;
    bool is_in_stack;
    ParsedInput parsed;
} cell;


void initialize_parsed_input(ParsedInput* input) {
    if (input == NULL) return;
    
    // Initialize target coordinates
    input->target[0] = 0;
    input->target[1] = 0;

    
    // Initialize all union members to 0
    // We can use memset since we want all bytes to be 0
    memset(&input->content, 0, sizeof(input->content));
    // Initialize expression type
    input->expression_type = -1;
}




bool valid_input(short_int rows, short_int cols){
    if(rows < 1 || rows > 999 || cols < 1 || cols > 18278){
        return false;
    }
    return true;
}

void initialize_sheet(cell ***sheet, short_int rows, short_int cols){
    // Allocate array of pointers to rows
    *sheet = (cell **)malloc(rows * sizeof(cell *));
    if(*sheet == NULL){
        return;
    }

    // Allocate array of cells for each row
    for(short_int i = 0; i < rows; i++){
        (*sheet)[i] = (cell *)malloc(cols * sizeof(cell));
        if((*sheet)[i] == NULL){
            // Free previously allocated memory if allocation fails
            for(short_int j = 0; j < i; j++){
                free((*sheet)[j]);
            }
            free(*sheet);
            return;
        }
    }

    // Initialize each cell
    for(short_int i = 0; i < rows; i++){
        for(short_int j = 0; j < cols; j++){
            (*sheet)[i][j].value = 0;
            (*sheet)[i][j].row = i;
            (*sheet)[i][j].col = j;
            (*sheet)[i][j].dependencies = NULL;
            (*sheet)[i][j].is_dirty = false;
            (*sheet)[i][j].is_in_stack = false;
            initialize_parsed_input(&(*sheet)[i][j].parsed);
        }
    }
}


void free_dependencies(Node *dependencies){
    Node *temp = dependencies;
    while(temp != NULL){
        Node *next = temp->next;        free(temp);
        temp = next;
    }
}

void free_sheet(cell **sheet, short_int row, short_int col){
    for(short_int i=0; i<row; i++){
        for(short_int j=0; j<col; j++){
            free_dependencies(sheet[i][j].dependencies);
        }
        free(sheet[i]);
    }
    free(sheet);
}


short_int min(short_int a, short_int b){
    return a < b ? a : b;
}

short_int max(short_int a, short_int b){
    return a > b ? a : b;
}

// Check if a string is a valid cell reference (e.g., "A1", "AA10")
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

// Parse a cell reference (e.g., "AA10" -> row=9, col=26)
short_int parse_cell(const char *cell, int *value, short_int sheet_rows, short_int sheet_cols) {
    if (!is_valid_cell(cell)) return 0;

    short_int i = 0;
    while (isalpha(cell[i])) i++;

    if(i==strlen(cell)) return 0; // Invalid cell reference

    char col_part[16], row_part[16];
    strncpy(col_part, cell, i);
    col_part[i] = '\0';
    strcpy(row_part, cell + i);

    // Convert column part to index (e.g., "A"=0, "AA"=26)
    short_int col = 0, row = 0;
    for (short_int j = 0; col_part[j] != '\0'; j++) {
        col = col * 26 + (toupper(col_part[j]) - 'A' + 1);
    }
    if(col == 0) return 0; // Invalid column part
    col -= 1;
    if(row_part[0] == '\0') return 0; // Invalid row part
    for(short_int i = 1; row_part[i] != '\0'; i++) {
        if(!isdigit(row_part[i])) {
            return 0;
        }
    }
    row = atoi(row_part)-1; // Convert to 0-based indexing
    if(row < 0 || row >= sheet_rows || col < 0 || col >= sheet_cols) return 0; // Out of bounds
    *value = (row << 16) | col;
    return 1;
}


double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int string_to_nat(char *s){
    short_int cur = 0;
    short_int inval = 0;
    while (*s != '\0'){
        if (*s>'9' || *s<'0'){
            inval = 1;
            break;
        }
        cur*=10;
        cur+=(*s-'0');
        s++;
    }
    if (cur == 0) inval = 1;
    if (inval == 1) return -1;
    else return cur;

}