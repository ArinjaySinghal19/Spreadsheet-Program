#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    

typedef struct cell{
    int value;
    int row;
    int col;
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
        }
    }
}