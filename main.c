#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "graph_checker.h"

ParsedInput parsed;


int main(int argc, char* argv[]) {
    if (argc !=3){
        printf("invalid input\n");
        return 0;
    }
    short_int rows, cols;
    rows = string_to_nat(argv[1]);
    cols = string_to_nat(argv[2]);
    if (rows == -1 || cols == -1){
        printf("invalid input\n");
        return 0;
    }

    double start = get_time();

    if(!valid_input(rows, cols)){
        printf("Invalid input\n");
        return 0;
    }
    short_int sr = 0, sc = 0, toggle_display = 1;
    cell **sheet;
    initialize_sheet(&sheet, rows, cols);
    if(sheet == NULL){
        printf("Memory allocation failed\n");
        return 0;
    }
    display_sheet(&sheet, rows, cols, toggle_display, sr, sc);
    double end = get_time();
    double time_taken = end - start;
    printf("[%.1f] (ok) > ", time_taken);
    char input[256];
    while(1){
        for(short_int i=0; i<256; i++) input[i]='\0';
        fgets(input, sizeof(input), stdin);
        start = get_time();
        if(strcmp(input, "q\n")==0 || strcmp(input, "Q\n")==0) {
            break;
        }
        
        short_int status = parse_input(input, &parsed, rows, cols);

        if(status>=6 && status<=11){
            if(!process_display(status, &toggle_display, &sr, &sc, rows, cols)){
                status=0;
            }else{
                status = 2;
            }
        }
        if(status == 12){
            if(input[strlen(input)-1] == '\n'){
                input[strlen(input)-1] = '\0';
            }
            if(!parse_cell(input + 10, &sr, &sc, rows, cols)){
                status=0;
            }else{
                status = 2;
                status = 2;
            }
        }
        if(status==0 || status == 2){
            display_sheet(&sheet, rows, cols, toggle_display, sr, sc);
            end = get_time();
            time_taken = end - start;
            if(status==0){
                printf("[%.1f] (Invalid Input) > ", time_taken);
            }else{
                printf("[%.1f] (ok) > ", time_taken);
            }
            continue;
        }
        short_int op_row = parsed.target[0];
        short_int op_col = parsed.target[1];
        ParsedInput previous_parsed;
        previous_parsed = sheet[op_row][op_col].parsed;
        sheet[op_row][op_col].parsed = parsed;

        short_int success = change(sheet, op_row, op_col, previous_parsed);
        if(!success){
            display_sheet(&sheet, rows, cols, toggle_display, sr, sc);
            end = get_time();
            time_taken = end - start;
            printf("[%.1f] (Cycle Detected) > ", time_taken);
            continue;
        }

        display_sheet(&sheet, rows, cols, toggle_display, sr, sc);

        end = get_time();
        time_taken = end - start;

        printf("[%.1f] (ok) > ", time_taken);
    }

    free_sheet(sheet, rows, cols);

    return 0;
}