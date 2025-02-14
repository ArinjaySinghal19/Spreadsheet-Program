#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include "initializer.h"

char colStr[4];

void int2col(int col) {
    char ans[4];
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
    for(int j = 0; j < i; j++) {
        colStr[j] = ans[i - j - 1];
    }
    colStr[i] = '\0';
    return;
    
}

void display_sheet(cell *** sheet, int rows, int cols, int toggle_display, int sr, int sc){
    if(toggle_display == 0){
        return;
    }
    printf("\t");
    int ec = min(sc + 10, cols);
    int er = min(sr + 10, rows);
    for(int j=sc; j<ec; j++){
        int2col(j+1);
        printf("%s\t", colStr);
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

int process_display(int status, int *toggle_display, int *sr, int *sc, int rows, int  cols) {
    if(status == 6) *sr = max(0, *sr - 10);
    else if(status == 7) *sc = max(0, *sc - 10);
    else if(status == 8) {
        *sr = *sr + 10;
        int er = min(*sr + 10, rows);
        *sr = max(er - 10, 0);
    }
    else if(status == 9) {
        *sc = *sc + 10;
        int ec = min(*sc + 10, cols);
        *sc = max(ec - 10, 0);
    }
    else if(status == 10) {
        *toggle_display = 0;
    }else if(status == 11) {
        *toggle_display = 1;
    }else{
        return 0;
    }
    return 1;
}