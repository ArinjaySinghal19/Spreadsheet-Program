#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node{
    int row;
    int col;
    struct Node *next;
} Node;

typedef struct {
    int target[2]; // Target cell (row, col)
    int expression_type; // 0 for value, 1 for expression, 2 for function
    int is_sleep; // 1 if SLEEP function, 0 otherwise
    int sleep_value[2]; // Sleep value (if is_sleep=1)
    int value[2]; // Value (if expression_type=0)
    int expression_cell_1[2]; // First cell in expression
    int expression_cell_2[2]; // Second cell in expression
    char expression_operator; // Operator in expression ( +, -, *, /)
    char function_operator; 
    int function_range[4]; // Function range (start row, start col, end row, end col)
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

void free_sheet(cell **sheet, int row, int col){
    for(int i=0; i<row; i++){
        for(int j=0; j<col; j++){
            free_dependencies(sheet[i][j].dependencies);
        }
        free(sheet[i]);
    }
    free(sheet);
}


int min(int a, int b){
    return a < b ? a : b;
}

