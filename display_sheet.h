#include<stdio.h>
#include<string.h>
// #include<initializer.h>
// #include "input_parser.h"

// display_sheet(sheet, rows, cols);

void display_sheet(cell *** sheet, int rows, int cols){
    printf("\t");
    for(int j=0; j<cols; j++){
        printf("%c   ", j+'A');
    }
    printf("\n");
    for(int i = 0; i < rows; i++){
        printf("%d\t", i+1);
        for(int j = 0; j < cols; j++){
            printf("%d   ", (*sheet)[i][j].value);
        }
        printf("\n");
    }
    return;
}