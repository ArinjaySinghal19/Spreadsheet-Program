#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include "initializer.h"
#include "input_parser.h"
#include "input_processing.h"
#include "display_sheet.h"

ParsedInput parsed;

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
        if(!status){
            display_sheet(&sheet, rows, cols);
            end = clock();
            time = (end - start) / CLOCKS_PER_SEC;
            printf("[%.2fms] (Invalid Input) > ", time);
            continue;
        }

        process_input(&parsed, &sheet, rows, cols);

        display_sheet(&sheet, rows, cols);

        end = clock();

        time = (end - start) / CLOCKS_PER_SEC;

        printf("[%.2fms] (ok) > ", time);

    }

    return 0;
}
