#include<stdio.h>
#include<string.h>
#include<stdlib.h>

char* int2col(int col) {
    char* ans = (char*)malloc(4 * sizeof(char));
    int i = 0;
    while (col > 0) {
        int rem = col % 26;
        if (rem == 0) {
            ans[i++] = 'Z';
            col = (col / 26) - 1;
        } else {
            ans[i++] = (rem - 1) + 'A';
            col = col / 26;
        }
    }
    char* colStr = (char*)malloc(4 * sizeof(char));
    for(int j = 0; j < i; j++) {
        colStr[j] = ans[i - j - 1];
    }
    colStr[i] = '\0';
    free(ans);
    return colStr;
    
}

void display_sheet(cell *** sheet, int rows, int cols){
    printf("\t");
    for(int j=0; j<cols; j++){
        printf("%s\t", int2col(j+1));
    }
    printf("\n");
    for(int i = 0; i < rows; i++){
        printf("%d\t", i+1);
        for(int j = 0; j < cols; j++){
            printf("%d\t", (*sheet)[i][j].value);
        }
        printf("\n");
    }
    return;
}