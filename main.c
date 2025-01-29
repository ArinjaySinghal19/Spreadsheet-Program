#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include "initializer.h"


int main(){

    int rows, cols;
    scanf("%d %d", &rows, &cols);
    getchar();

    if(!valid_input(rows, cols)){
        printf("Invalid input\n");
        return 0;
    }

    cell **sheet;
    initialize_sheet(&sheet, rows, cols);

    // while(1){
    //     double start = clock();

    //     char *input = (char *)malloc(100 * sizeof(char));
    //     fgets(input, 100, stdin);

    //     if(invalid_input(input)){
    //         display_sheet(sheet, rows, cols);
    //         double end = clock();
    //         double time = (end - start) / CLOCKS_PER_SEC;
    //         printf("[%.2f s] (Invalid Input) > ", time);
    //         continue;
    //     }

    //     process_input(&input, &sheet, rows, cols);

    //     display_sheet(sheet, rows, cols);

    //     double end = clock();

    //     double time = (end - start) / CLOCKS_PER_SEC;

    //     printf("[%.2f s] (ok) > ", time);

    // }

    return 0;
}
