#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

typedef struct Node{
    int row;
    int col;
    struct Node *next;
} Node;

typedef struct {
    int target[2]; // Target cell (row, col)
    unsigned int expression_type : 2; // 0 for value, 1 for expression, 2 for function, 3 for sleep
    union {
        struct {
            int value[2]; // Value (if expression_type=0)
        } value_data;

        struct {
            int expression_cell_1[2]; // First cell in expression
            int expression_cell_2[2]; // Second cell in expression
            char expression_operator; // Operator in expression ( +, -, *, /)
        } expression_data;

        struct {
            char function_operator; 
            int function_range[4]; // Function range (start row, start col, end row, end col)
        } function_data;

        struct {
            int sleep_value[2]; // Sleep value (if expression_type=3)
        } sleep_data;
    } content;
} ParsedInput;

typedef struct cell{
    int value;
    int row;
    int col;
    Node *dependencies;
    Node *depends_on;
    bool is_dirty;
    int dirty_parents;
    ParsedInput parsed;
} cell;


void initialize_parsed_input(ParsedInput* input) {
    if (input == NULL) return;
    
    // Initialize target coordinates
    input->target[0] = 0;
    input->target[1] = 0;
    
    // Initialize expression type
    input->expression_type = 0;
    
    // Initialize all union members to 0
    // We can use memset since we want all bytes to be 0
    memset(&input->content, 0, sizeof(input->content));
}




bool valid_input(int rows, int cols){
    if(rows < 1 || rows > 999 || cols < 1 || cols > 18278){
        return false;
    }
    return true;
}

void initialize_sheet(cell ***sheet, int rows, int cols){
    // Allocate array of pointers to rows
    *sheet = (cell **)malloc(rows * sizeof(cell *));
    if(*sheet == NULL){
        return;
    }

    // Allocate array of cells for each row
    for(int i = 0; i < rows; i++){
        (*sheet)[i] = (cell *)malloc(cols * sizeof(cell));
        if((*sheet)[i] == NULL){
            // Free previously allocated memory if allocation fails
            for(int j = 0; j < i; j++){
                free((*sheet)[j]);
            }
            free(*sheet);
            return;
        }
    }

    // Initialize each cell
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            (*sheet)[i][j].value = 0;
            (*sheet)[i][j].row = i;
            (*sheet)[i][j].col = j;
            (*sheet)[i][j].dependencies = NULL;
            (*sheet)[i][j].depends_on = NULL;
            (*sheet)[i][j].is_dirty = false;
            (*sheet)[i][j].dirty_parents = 0;
            initialize_parsed_input(&(*sheet)[i][j].parsed);
        }
    }
}


void free_dependencies(Node *dependencies){
    Node *temp = dependencies;
    while(temp != NULL){
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
}

void free_depends_on(Node *depends_on){
    Node *temp = depends_on;
    while(temp != NULL){
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
}

void free_sheet(cell **sheet, int row, int col){
    for(int i=0; i<row; i++){
        for(int j=0; j<col; j++){
            free_dependencies(sheet[i][j].dependencies);
            free_depends_on(sheet[i][j].depends_on);
        }
        free(sheet[i]);
    }
    free(sheet);
}


int min(int a, int b){
    return a < b ? a : b;
}

int max(int a, int b){
    return a > b ? a : b;
}

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
int parse_cell(const char *cell, int *row, int *col, int sheet_rows, int sheet_cols) {
    int original_row = *row;
    int original_col = *col;
    if (!is_valid_cell(cell)) return 0;

    int i = 0;
    while (isalpha(cell[i])) i++;

    if(i==strlen(cell)) return 0; // Invalid cell reference

    char col_part[16], row_part[16];
    strncpy(col_part, cell, i);
    col_part[i] = '\0';
    strcpy(row_part, cell + i);

    // Convert column part to index (e.g., "A"=0, "AA"=26)
    *col = 0;
    for (int j = 0; col_part[j] != '\0'; j++) {
        *col = *col * 26 + (toupper(col_part[j]) - 'A' + 1);
    }
    if(*col == 0) return 0; // Invalid column part
    *col -= 1;
    if(row_part[0] == '\0') return 0; // Invalid row part
    for(int i = 1; row_part[i] != '\0'; i++) {
        if(!isdigit(row_part[i])) {
            return 0;
        }
    }
    *row = atoi(row_part)-1; // Convert to 0-based indexing
    if(*row < 0 || *row >= sheet_rows || *col < 0 || *col >= sheet_cols){
        *row = original_row;
        *col = original_col;
        return 0; // Invalid row index
    }
    return 1;
}


double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}