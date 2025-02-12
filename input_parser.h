#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "display_sheet.h"

// Define constants for program flow
#define EXIT_PROGRAM 1
#define CONTINUE_PROGRAM 0

//Values are stores in arrays of two, if first indice is -1, then the second indice is the number.
//If the first indice is not -1, then it's the row, seocond indice is the column.

// Structure to hold parsed input

int parse_value(const char *cell, int *row, int *col) {
    if (parse_cell(cell, row, col)) {
        return 1;
    }
    //check if string is a number
    if(!isdigit(cell[0])) {
        return 0;
    }
    for(int i = 0; cell[i] != '\0'; i++) {
        if(!isdigit(cell[i])) {
            return 0;
        }
    }
    *row = -1;
    *col = atoi(cell);
    return 1;
}

int evaluate_range(ParsedInput *parsed, const char *input) {
    // Parse the range (e.g., A1:A10)
    int i = 0;
    int j = 0;
    char end_cell[32], start_cell[32];
    if(sscanf(input, "%31[^:]:%31s", start_cell, end_cell) != 2) {
    
        return 0; // Invalid range format
    }
    if(end_cell[strlen(end_cell)-1] == ')') {
        end_cell[strlen(end_cell)-1] = '\0';
    }else{
        return 0; // Invalid range format
    }
    if(start_cell[0]!='(') return 0; // Invalid range format

    if (!parse_cell(&start_cell[1], &parsed->content.function_data.function_range[0], &parsed->content.function_data.function_range[1])) {
        return 0; // Invalid start cell
    }
    if (!parse_cell(end_cell, &parsed->content.function_data.function_range[2], &parsed->content.function_data.function_range[3])) {
        return 0; // Invalid end cell
    }
    return 1;
}

int handle_expression(ParsedInput *parsed, char *expr) {
    // Expressions can be of broadly 3 types: values, arithmetic expressions, functions.
    // Functions can be MIN(Range), MAX(Range), AVG(Range), SUM(Range), STDEV(Range), SLEEP(Value).

    // Check if the expression is one of the functions
    if (strncmp(expr, "MIN", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 0;
        evaluate_range(parsed, expr+3);
        return 1;
    }
    if (strncmp(expr, "MAX", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 1;
        evaluate_range(parsed, expr+3);
        return 1;
    }
    if (strncmp(expr, "AVG", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 2;
        evaluate_range(parsed, expr+3);
        return 1;
    }
    if (strncmp(expr, "SUM", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 3;
        evaluate_range(parsed, expr+3);
        return 1;
    }
    if (strncmp(expr, "STDEV", 5) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 4;
        evaluate_range(parsed, expr+5);
        return 1;
    }
    if (strncmp(expr, "SLEEP", 5) == 0) {
        parsed->expression_type = 3;
        if(expr[5] != '(' || expr[strlen(expr)-1] != ')') {
            return 0; // Invalid SLEEP function format
        }
        expr[strlen(expr)-1] = '\0';
        if(!parse_value(expr+6, &parsed->content.sleep_data.sleep_value[0], &parsed->content.sleep_data.sleep_value[1])) {
            return 0; // Invalid value
        }
        return 1;
    }

    // Check if the expression is an arithmetic expression: check for the presence of operators (+, -, *, /)
    if (strchr(expr, '+') || strchr(expr, '-') || strchr(expr, '*') || strchr(expr, '/')) {
        parsed->expression_type = 1;
        int i = 0;
        while (expr[i] != '+' && expr[i] != '-' && expr[i] != '*' && expr[i] != '/') i++;
        parsed->content.expression_data.expression_operator = expr[i];
        //divide expression into two values
        expr[i] = '\0';
        if (!parse_value(expr, &parsed->content.expression_data.expression_cell_1[0], &parsed->content.expression_data.expression_cell_1[1])) {
            return 0; // Invalid first cell
        }
        if (!parse_value(expr+i+1, &parsed->content.expression_data.expression_cell_2[0], &parsed->content.expression_data.expression_cell_2[1])) {
            return 0; // Invalid second cell
        }
        return 1;
    }

    // If not a function or expression, then it must be a value
    parsed->expression_type = 0;
    if (!parse_value(expr, &parsed->content.value_data.value[0], &parsed->content.value_data.value[1])) {
        return 0; // Invalid value
    }
    return 1;
}

int handle_display(const char *input) {
    if(strcmp(input, "disable_output\n") == 0) {
        return 10;
    }else if(strcmp(input, "enable_output\n") == 0) {
        return 11;
    }
    else if(strcmp(input, "w\n") == 0) {
        return 6;
    }else if(strcmp(input, "a\n") == 0) {
        return 7;
    }else if(strcmp(input, "s\n") == 0) {
        return 8;
    }else if(strcmp(input, "d\n") == 0) {
        return 9;
    }else if(strncmp(input, "scroll_to", 9) == 0) {
        return 12;
    }else{
        return 0;
    }
}


// Parse the input into a ParsedInput structure
int parse_input(char *input, ParsedInput *parsed) {
    char cell[32], expr[128];
    

    // Extract cell reference and value/expression
    if (sscanf(input, "%31[^=]=%127s", cell, expr) != 2) {
        return (handle_display(input));
    }
    
    if(expr[strlen(expr)-1] == '\n') {
        expr[strlen(expr)-1] = '\0';
    }
    // Parse the cell reference
    if (!parse_cell(cell, &parsed->target[0], &parsed->target[1])) {
        return 0; // Invalid cell reference
    }

    if(!handle_expression(parsed, expr)) {
        return 0; // Invalid expression
    }
    return 1; // Successfully parsed
}

// Handle user input
int handle_input(const char *input) {

    // Handle formulas and assignments
    if (input[0] == 'q') {
        return EXIT_PROGRAM; // Exit program
    }
    ParsedInput parsed;
    // initialize parsed
    if (parse_input(input, &parsed)) {
        // Print the parsed input
        printf("Target cell: (%d, %d)\n", parsed.target[0], parsed.target[1]);
        printf("Expression type: %d\n", parsed.expression_type);
        if(parsed.expression_type == 3) {
            printf("Function: SLEEP\n");
            printf("Value: (%d, %d)\n", parsed.content.sleep_data.sleep_value[0], parsed.content.sleep_data.sleep_value[1]);
        }        
        else if (parsed.expression_type == 0) {
            printf("Value: (%d, %d)\n", parsed.content.value_data.value[0], parsed.content.value_data.value[1]);
        } else if (parsed.expression_type == 1) {
            printf("Expression: (%d, %d) %c (%d, %d)\n", parsed.content.expression_data.expression_cell_1[0], parsed.content.expression_data.expression_cell_1[1],
                   parsed.content.expression_data.expression_operator, parsed.content.expression_data.expression_cell_2[0], parsed.content.expression_data.expression_cell_2[1]);
        } else if (parsed.expression_type == 2) {
            printf("Function: %d\n", parsed.content.function_data.function_operator);
            printf("Range: (%d, %d) to (%d, %d)\n", parsed.content.function_data.function_range[0], parsed.content.function_data.function_range[1], parsed.content.function_data.function_range[2], parsed.content.function_data.function_range[3]);
        }
        return CONTINUE_PROGRAM;
    }

    // If no valid input, show an error
    printf("Error: Unrecognized command\n");
    return CONTINUE_PROGRAM;
}