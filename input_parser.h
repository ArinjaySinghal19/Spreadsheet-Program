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

short_int parse_value(const char *cell, int *value, short_int sheet_rows, short_int sheet_cols) { //returns 0: error, 1: value, 2: cell
    if (parse_cell(cell, value, sheet_rows, sheet_cols)) {
        return 2; 
    }
    //check if string is a number
    short_int sign = 1;
    short_int iter = 0;
    if(cell[0] == '-'){
        sign = -1;
        iter = 1;
    }else if(cell[0] == '+'){
        iter = 1;
    }
    if(!isdigit(cell[iter])) {
        return 0;
    }
    for(short_int i = iter; cell[i] != '\0'; i++) {
        if(!isdigit(cell[i])) {
            return 0;
        }
    }
    *value = sign*atoi(cell);
    return 1;
}

short_int evaluate_range(ParsedInput *parsed, const char *input, short_int sheet_rows, short_int sheet_cols) {
    // Parse the range (e.g., A1:A10)
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

    if (!parse_cell(&start_cell[1], &parsed->content.function_data.function_range[0], sheet_rows, sheet_cols)) {
        return 0; // Invalid start cell
    }
    if (!parse_cell(end_cell, &parsed->content.function_data.function_range[1], sheet_rows, sheet_cols)) {
        return 0; // Invalid end cell
    }
    short_int start_row = (parsed->content.function_data.function_range[0]) >> 16;
    short_int start_col = (parsed->content.function_data.function_range[0]) & 0xFFFF;
    short_int end_row = (parsed->content.function_data.function_range[1]) >> 16;
    short_int end_col = (parsed->content.function_data.function_range[1]) & 0xFFFF;
    if(start_row > end_row || start_col > end_col) return 0; // Invalid range
    return 1;
}

short_int handle_expression(ParsedInput *parsed, char *expr, short_int sheet_rows, short_int sheet_cols) {
    // Expressions can be of broadly 3 types: values, arithmetic expressions, functions.
    // Functions can be MIN(Range), MAX(Range), AVG(Range), SUM(Range), STDEV(Range), SLEEP(Value).

    // Check if the expression is one of the functions
    if (strncmp(expr, "MIN", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 0;
        if(!evaluate_range(parsed, expr+3, sheet_rows, sheet_cols)) return 0;
        return 1;
    }
    if (strncmp(expr, "MAX", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 1;
        if(!evaluate_range(parsed, expr+3, sheet_rows, sheet_cols)) return 0;
        return 1;
    }
    if (strncmp(expr, "AVG", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 2;
        if(!evaluate_range(parsed, expr+3, sheet_rows, sheet_cols)) return 0;
        return 1;
    }
    if (strncmp(expr, "SUM", 3) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 3;
        if(!evaluate_range(parsed, expr+3, sheet_rows, sheet_cols)) return 0;
        return 1;
    }
    if (strncmp(expr, "STDEV", 5) == 0) {
        parsed->expression_type = 2;
        parsed->content.function_data.function_operator = 4;
        if(!evaluate_range(parsed, expr+5, sheet_rows, sheet_cols)) return 0;
        return 1;
    }
    if (strncmp(expr, "SLEEP", 5) == 0) {
        parsed->expression_type = 3;
        if(expr[5] != '(' || expr[strlen(expr)-1] != ')') {
            return 0; // Invalid SLEEP function format
        }
        expr[strlen(expr)-1] = '\0';
        short_int status = parse_value(expr+6, &parsed->content.sleep_data.sleep_value, sheet_rows, sheet_cols);
        if(status == 0) return 0; // Invalid sleep value
        if(status == 1){
            parsed->content.sleep_data.is_value = 1;
        }
        if(status == 2){
            parsed->content.sleep_data.is_value = 0;
        }
        return 1;
    }
    // Check if the expression is an arithmetic expression: check for the presence of operators (+, -, *, /)
    if (strchr(expr, '+') || strchr(expr, '-') || strchr(expr, '*') || strchr(expr, '/')) {
        parsed->expression_type = 1;
        short_int i = 0;
        if(expr[i] == '-') i++;
        else if(expr[i] == '+') i++;
        while (expr[i] != '+' && expr[i] != '-' && expr[i] != '*' && expr[i] != '/') i++;
        parsed->content.expression_data.expression_operator = expr[i];
        //divide expression into two values
        expr[i] = '\0';
        short_int status = parse_value(expr, &parsed->content.expression_data.expression_cell[0], sheet_rows, sheet_cols);
        if(status == 0) return 0; // Invalid first cell
        if(status == 1){
            parsed->content.expression_data.is_value_1 = 1;
        }
        if(status == 2){
            parsed->content.expression_data.is_value_1 = 0;
        }
        status = parse_value(expr+i+1, &parsed->content.expression_data.expression_cell[1], sheet_rows, sheet_cols);
        if(status == 0) return 0; // Invalid second cell
        if(status == 1){
            parsed->content.expression_data.is_value_2 = 1;
        }
        
        return 1;
    }

    // If not a function or expression, then it must be a value
    parsed->expression_type = 0;
    short_int status = parse_value(expr, &parsed->content.value_data.value, sheet_rows, sheet_cols);
    if(status == 0) return 0; // Invalid value
    if(status == 1){
        parsed->content.value_data.is_value = 1;
    }
    if(status == 2){
        parsed->content.value_data.is_value = 0;
    }
    return 1;
}

short_int handle_display(const char *input) {
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
short_int parse_input(const char *input, ParsedInput *parsed, short_int sheet_rows, short_int sheet_cols) {
    char cell[32], expr[128];
    

    // Extract cell reference and value/expression
    if (sscanf(input, "%31[^=]=%127s", cell, expr) != 2) {
        return (handle_display(input));
    }
    if(expr[strlen(expr)-1] == '\n') {
        expr[strlen(expr)-1] = '\0';
    }
    // Parse the cell reference
    int target_value = -1;
    if (!parse_cell(cell, &target_value, sheet_rows, sheet_cols)) {
        return 0; // Invalid cell reference
    }
    parsed->target[0] = (target_value) >> 16;
    parsed->target[1] = (target_value) & 0xFFFF;
    if(!handle_expression(parsed, expr, sheet_rows, sheet_cols)) {
        return 0; // Invalid expression
    }

    return 1; // Successfully parsed
}

// Handle user input
short_int handle_input(const char *input, short_int sheet_rows, short_int sheet_cols) {

    // Handle formulas and assignments
    if (input[0] == 'q') {
        return EXIT_PROGRAM; // Exit program
    }
    ParsedInput parsed;
    // initialize parsed
    if (parse_input(input, &parsed, sheet_rows, sheet_cols)) {
        // Prshort_int the parsed input
        printf("Target cell: (%d, %d)\n", parsed.target[0], parsed.target[1]);
        printf("Expression type: %d\n", parsed.expression_type);
        if(parsed.expression_type == 3) {
            printf("Function: SLEEP\n");
            printf("Value: (%d, %d)\n", parsed.content.sleep_data.sleep_value, parsed.content.sleep_data.is_value);
        }        
        else if (parsed.expression_type == 0) {
            printf("Value: (%d, %d)\n", parsed.content.value_data.value, parsed.content.value_data.is_value);
        } else if (parsed.expression_type == 1) {
            printf("Expression: (%d, %d) %c (%d, %d)\n", (parsed.content.expression_data.expression_cell[0] >> 16), (parsed.content.expression_data.expression_cell[0] & 0xFFFF), parsed.content.expression_data.expression_operator, (parsed.content.expression_data.expression_cell[1] >> 16), (parsed.content.expression_data.expression_cell[1] & 0xFFFF));
        } else if (parsed.expression_type == 2) {
            printf("Function: %d\n", parsed.content.function_data.function_operator);
            printf("Range: (%d, %d) to (%d, %d)\n", (parsed.content.function_data.function_range[0] >> 16), (parsed.content.function_data.function_range[0] & 0xFFFF), (parsed.content.function_data.function_range[1] >> 16), (parsed.content.function_data.function_range[1] & 0xFFFF));  
        }
        return CONTINUE_PROGRAM;
    }

    // If no valid input, show an error
    printf("Error: Unrecognized command\n");
    return CONTINUE_PROGRAM;
}
