#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include "initializer.h"


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

void display_sheet(cell *** sheet, int rows, int cols, int toggle_display, int sr, int sc){
    if(toggle_display == 0){
        return;
    }
    printf("\t");
    int ec = min(sc + 10, cols);
    int er = min(sr + 10, rows);
    for(int j=sc; j<ec; j++){
        printf("%s\t", int2col(j+1));
    }
    printf("\n");
    for(int i = sr; i < er; i++){
        printf("%d\t", i+1);
        for(int j = sc; j < ec; j++){
            if((*sheet)[i][j].value == INT32_MIN){
                printf("ERR\t");
            }else{
                printf("%d\t", (*sheet)[i][j].value);
            }
        }
        printf("\n");
    }
    return;
}

//int process_display(status, &toggle_display, &sr, &sc, &er, &ec, rows, cols);
int process_display(int status, int *toggle_display, int *sr, int *sc, int rows, int  cols) {
    if(status == 6) {
        *sr = *sr - 10;
        if(*sr < 0) {
            *sr = 0;
        }
    }else if(status == 7) {
        *sc = *sc - 10;
        if(*sc < 0) {
            *sc = 0;
        }
    }else if(status == 8) {
        *sr = *sr + 10;
        if(*sr >= rows) {
            *sr = rows - 10;
        }
    }else if(status == 9) {
        *sc = *sc + 10;
        if(*sc >= cols) {
            *sc = cols - 10;
        }
    }else if(status == 10) {
        *toggle_display = 0;
    }else if(status == 11) {
        *toggle_display = 1;
    }else{
        return 0;
    }
    return 1;
}