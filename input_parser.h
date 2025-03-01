#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "display_sheet.h"

/* Program flow constants */
#define EXIT_PROGRAM 1
#define CONTINUE_PROGRAM 0

/**
 * Parse a value from string into either a number or cell reference
 * Returns: 0 (error), 1 (numeric value), 2 (cell reference)
 */
short_int parse_value(const char *cell, int *value, short_int sheet_rows, short_int sheet_cols) {
    // Try parsing as cell reference first
    if (parse_cell(cell, value, sheet_rows, sheet_cols)) {
        return 2;  // Cell reference
    }
    
    // Try parsing as a number
    short_int sign = 1;
    short_int iter = 0;
    
    // Handle sign prefix
    if (cell[0] == '-') {
        sign = -1;
        iter = 1;
    } else if (cell[0] == '+') {
        iter = 1;
    }
    
    // Validate first digit
    if (!isdigit(cell[iter])) {
        return 0;  // Invalid format
    }
    
    // Validate remaining digits
    for (short_int i = iter; cell[i] != '\0'; i++) {
        if (!isdigit(cell[i])) {
            return 0;  // Invalid format
        }
    }
    // Convert to integer
    *value = sign * atoi(cell + iter);
    return 1;  // Numeric value
}

/**
 * Evaluate a cell range (e.g., A1:B5) for functions
 */
short_int evaluate_range(ParsedInput *parsed, const char *input, short_int sheet_rows, short_int sheet_cols) {
    char start_cell[32], end_cell[32];
    
    // Parse range format "(<start_cell>:<end_cell>)"
    if (sscanf(input, "%31[^:]:%31s", start_cell, end_cell) != 2) {
        return 0;  // Invalid range format
    }
    
    // Validate closing parenthesis
    if (end_cell[strlen(end_cell) - 1] == ')') {
        end_cell[strlen(end_cell) - 1] = '\0';
    } else {
        return 0;  // Missing closing parenthesis
    }
    
    // Validate opening parenthesis
    if (start_cell[0] != '(') {
        return 0;  // Missing opening parenthesis
    }
    
    // Parse the start cell (skip the opening parenthesis)
    if (!parse_cell(&start_cell[1], &parsed->content.function_data.function_range[0], sheet_rows, sheet_cols)) {
        return 0;  // Invalid start cell
    }
    
    // Parse the end cell
    if (!parse_cell(end_cell, &parsed->content.function_data.function_range[1], sheet_rows, sheet_cols)) {
        return 0;  // Invalid end cell
    }
    
    // Extract row and column for range validation
    short_int start_row = (parsed->content.function_data.function_range[0]) >> 16;
    short_int start_col = (parsed->content.function_data.function_range[0]) & 0xFFFF;
    short_int end_row = (parsed->content.function_data.function_range[1]) >> 16;
    short_int end_col = (parsed->content.function_data.function_range[1]) & 0xFFFF;
    // Validate range coordinates
    if (start_row > end_row || start_col > end_col) {
        return 0;  // Invalid range (start must be before end)
    }
    
    return 1;  // Valid range
}

/**
 * Process an expression (value, arithmetic, or function)
 */
short_int handle_expression(ParsedInput *parsed, char *expr, short_int sheet_rows, short_int sheet_cols) {
    // Functions: MIN, MAX, AVG, SUM, STDEV, SLEEP
    
    // Check if expression is MIN function
    if (strncmp(expr, "MIN", 3) == 0) {
        parsed->expression_type = '2';
        parsed->operator = '0';
        if (!evaluate_range(parsed, expr + 3, sheet_rows, sheet_cols)) {
            return 0;  // Invalid range in MIN function
        }
        return 1;
    }
    
    // Check if expression is MAX function
    if (strncmp(expr, "MAX", 3) == 0) {
        parsed->expression_type = '2';
        parsed->operator = '1';
        if (!evaluate_range(parsed, expr + 3, sheet_rows, sheet_cols)) {
            return 0;  // Invalid range in MAX function
        }
        return 1;
    }
    
    // Check if expression is AVG function
    if (strncmp(expr, "AVG", 3) == 0) {
        parsed->expression_type = '2';
        parsed->operator = '2';
        if (!evaluate_range(parsed, expr + 3, sheet_rows, sheet_cols)) {
            return 0;  // Invalid range in AVG function
        }
        return 1;
    }
    
    // Check if expression is SUM function
    if (strncmp(expr, "SUM", 3) == 0) {
        parsed->expression_type = '2';
        parsed->operator = '3';
        if (!evaluate_range(parsed, expr + 3, sheet_rows, sheet_cols)) {
            return 0;  // Invalid range in SUM function
        }
        return 1;
    }
    
    // Check if expression is STDEV function
    if (strncmp(expr, "STDEV", 5) == 0) {
        parsed->expression_type = '2';
        parsed->operator = '4';
        if (!evaluate_range(parsed, expr + 5, sheet_rows, sheet_cols)) {
            return 0;  // Invalid range in STDEV function
        }
        return 1;
    }
    
    // Check if expression is SLEEP function
    if (strncmp(expr, "SLEEP", 5) == 0) {
        parsed->expression_type = '3';
        
        // Validate SLEEP function format
        if (expr[5] != '(' || expr[strlen(expr) - 1] != ')') {
            return 0;  // Invalid SLEEP function format
        }
        
        // Remove closing parenthesis for parsing
        expr[strlen(expr) - 1] = '\0';
        
        // Parse sleep value
        short_int status = parse_value(expr + 6, &parsed->content.sleep_data.sleep_value, sheet_rows, sheet_cols);
        if (status == 0) {
            return 0;  // Invalid sleep value
        }
        
        // Set is_value flag based on whether it's a constant or cell reference
        parsed->is_value_1 = (status == 1);
        
        return 1;
    }
    
    parsed->expression_type = '0';
    

    // If not a function or expression, then it must be a value
    short_int status = parse_value(expr, &parsed->content.value_data.value, sheet_rows, sheet_cols);
    if (status == 1) {
        parsed->is_value_1 = 1;
        return 1;  // Successfully parsed value
    }
    if(status == 2) {
        parsed->is_value_1 = 0;
        return 1;  // Successfully parsed value
    }
    

    // Check if expression is an arithmetic expression containing operators (+, -, *, /)
    if (strchr(expr, '+') || strchr(expr, '-') || strchr(expr, '*') || strchr(expr, '/')) {
        parsed->expression_type = '1';
        short_int i = 0;
        
        // Skip leading sign for negative numbers
        if (expr[i] == '-' || expr[i] == '+') {
            i++;
        }
        
        // Find the operator
        while (expr[i] != '+' && expr[i] != '-' && expr[i] != '*' && expr[i] != '/') {
            i++;
        }
        
        parsed->operator = expr[i];
        
        // Split expression at operator
        expr[i] = '\0';
        
        // Parse first operand
        short_int status = parse_value(expr, &parsed->content.expression_data.expression_cell[0], sheet_rows, sheet_cols);
        if (status == 0) {
            return 0;  // Invalid first operand
        }
        parsed->is_value_1 = (status == 1);
        
        // Parse second operand
        status = parse_value(expr + i + 1, &parsed->content.expression_data.expression_cell[1], sheet_rows, sheet_cols);
        if (status == 0) {
            return 0;  // Invalid second operand
        }
        parsed->is_value_2 = (status == 1);
        
        return 1;
    }
    
    return 0;  // Successfully parsed value
}

/**
 * Handle display commands for spreadsheet navigation
 */
short_int handle_display(const char *input) {
    if (strcmp(input, "disable_output\n") == 0) {
        return 10;
    } else if (strcmp(input, "enable_output\n") == 0) {
        return 11;
    } else if (strcmp(input, "w\n") == 0) {
        return 6;  // Move up
    } else if (strcmp(input, "a\n") == 0) {
        return 7;  // Move left
    } else if (strcmp(input, "s\n") == 0) {
        return 8;  // Move down
    } else if (strcmp(input, "d\n") == 0) {
        return 9;  // Move right
    } else if (strncmp(input, "scroll_to", 9) == 0) {
        return 12;  // Scroll to specific cell
    } else {
        return 0;  // Unrecognized display command
    }
}

/**
 * Parse user input into a structured ParsedInput
 */
short_int parse_input(const char *input, ParsedInput *parsed, short_int sheet_rows, short_int sheet_cols) {
    char cell[32], expr[128];
    
    // Check if input is a cell assignment (contains =)
    if (sscanf(input, "%31[^=]=%127s", cell, expr) != 2) {
        return handle_display(input);  // Not an assignment, try as display command
    }
    
    // Remove trailing newline if present
    if (expr[strlen(expr) - 1] == '\n') {
        expr[strlen(expr) - 1] = '\0';
    }
    
    // Parse the target cell reference
    int target_value = -1;
    if (!parse_cell(cell, &target_value, sheet_rows, sheet_cols)) {
        return 0;  // Invalid cell reference
    }
    
    // Set target cell coordinates
    parsed->target[0] = (target_value) >> 16;      // Row
    parsed->target[1] = (target_value) & 0xFFFF;   // Column
    
    // Process the expression part
    if (!handle_expression(parsed, expr, sheet_rows, sheet_cols)) {
        return 0;  // Invalid expression
    }
    
    return 1;  // Successfully parsed input
}

/**
 * Process user input and determine next action
 */
short_int handle_input(const char *input, short_int sheet_rows, short_int sheet_cols) {
    // Check for quit command
    if (input[0] == 'q') {
        return EXIT_PROGRAM;
    }
    
    // Parse the input
    ParsedInput parsed;
    initialize_parsed_input(&parsed);  // Initialize with default values
    
    if (parse_input(input, &parsed, sheet_rows, sheet_cols)) {
        // Print parsed input details for debugging
        printf("Target cell: (%d, %d)\n", parsed.target[0], parsed.target[1]);
        printf("Expression type: %c\n", parsed.expression_type);
        
        // Print details based on expression type
        if (parsed.expression_type == '3') {
            printf("Function: SLEEP\n");
            printf("Value: (%d, %d)\n", parsed.content.sleep_data.sleep_value, 
                  parsed.is_value_1);
        } else if (parsed.expression_type == '0') {
            printf("Value: (%d, %d)\n", parsed.content.value_data.value, 
                  parsed.is_value_1);
        } else if (parsed.expression_type == '1') {
            printf("Expression: (%d, %d) %c (%d, %d)\n", 
                  (parsed.content.expression_data.expression_cell[0] >> 16), 
                  (parsed.content.expression_data.expression_cell[0] & 0xFFFF), 
                  parsed.operator,
                  (parsed.content.expression_data.expression_cell[1] >> 16), 
                  (parsed.content.expression_data.expression_cell[1] & 0xFFFF));
        } else if (parsed.expression_type == '2') {
            printf("Function: %c\n", parsed.operator);
            printf("Range: (%d, %d) to (%d, %d)\n", 
                  (parsed.content.function_data.function_range[0] >> 16), 
                  (parsed.content.function_data.function_range[0] & 0xFFFF), 
                  (parsed.content.function_data.function_range[1] >> 16), 
                  (parsed.content.function_data.function_range[1] & 0xFFFF));
        }
        
        return CONTINUE_PROGRAM;
    }
    
    // If no valid input was found, show an error
    printf("Error: Unrecognized command\n");
    return CONTINUE_PROGRAM;
}