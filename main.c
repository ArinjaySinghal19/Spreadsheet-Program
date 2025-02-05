#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include "graph_checker.h"

ParsedInput parsed;

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



int main(){

    int rows, cols;
    scanf("%d %d", &rows, &cols);
    getchar();
    double start = clock();

    if(!valid_input(rows, cols)){
        printf("Invalid input\n");
        return 0;
    }

    cell **sheet;
    initialize_sheet(&sheet, rows, cols);
    display_sheet(&sheet, rows, cols);
    double end = clock();
    double time = (end - start) / CLOCKS_PER_SEC;
    printf("[%.2fms] (ok) > ", time);
    while(1){

        char input[256];
        for(int i=0; i<256; i++) input[i]='\0';
        fgets(input, sizeof(input), stdin);
        start = clock();
        if(strcmp(input, "q\n")==0 || strcmp(input, "Q\n")==0) {
            return 0;
        }
        int status = parse_input(input, &parsed);
        sheet[parsed.target[0]][parsed.target[1]].parsed = parsed;
        if(!status){
            display_sheet(&sheet, rows, cols);
            end = clock();
            time = (end - start) / CLOCKS_PER_SEC;
            printf("[%.2fms] (Invalid Input) > ", time);
            continue;
        }

        change(sheet, parsed.target[0], parsed.target[1]);

        display_sheet(&sheet, rows, cols);

        end = clock();

        time = (end - start) / CLOCKS_PER_SEC;

        printf("[%.2fms] (ok) > ", time);

    }

    return 0;
}
