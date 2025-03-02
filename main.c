#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "graph_checker.h"

ParsedInput parsed;

int main(int argc, char* argv[]) {
    // Validate command line arguments
    if (argc != 3) {
        printf("Invalid input\n");
        return 0;
    }
    
    // Parse row and column dimensions
    short_int rows = string_to_nat(argv[1]);
    short_int cols = string_to_nat(argv[2]);
    
    if (rows == -1 || cols == -1) {
        printf("Invalid input\n");
        return 0;
    }

    double start = get_time();

    if (!valid_input(rows, cols)) {
        printf("Invalid input\n");
        return 0;
    }
    
    // Initialize application state
    bool toggle_display = 1;
    int sr_sc = 0;  // Combined scroll position (row in high bits, col in low bits)
    cell **sheet;
    
    // Allocate memory for the sheet
    initialize_sheet(&sheet, rows, cols);
    if (sheet == NULL) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    // Initial display of the sheet
    display_sheet(&sheet, rows, cols, toggle_display, (sr_sc >> 16), (sr_sc & 0xFFFF));
    
    double end = get_time();
    double time_taken = end - start;
    printf("[%.1f] (ok) > ", time_taken);
    
    // Main command processing loop
    char input[256];
    while (1) {
        // Clear input buffer
        for (short_int i = 0; i < 256; i++) {
            input[i] = '\0';
        }
        
        // Get user input
        fgets(input, sizeof(input), stdin);
        start = get_time();
        
        // Check for quit command
        if (strcmp(input, "q\n") == 0 || strcmp(input, "Q\n") == 0) {
            break;
        }
        
        // Parse the input command
        short_int status = parse_input(input, &parsed, rows, cols);

        // Handle display-related commands (status 6-11)
        if (status >= 6 && status <= 11) {
            if (!process_display(status, &toggle_display, &sr_sc, rows, cols)) {
                status = 0;  // Invalid input
            } else {
                status = 2;  // Valid command
            }
        }
        
        // Handle cell selection command
        if (status == 12) {
            // Remove trailing newline if present
            if (input[strlen(input) - 1] == '\n') {
                input[strlen(input) - 1] = '\0';
            }
            
            if (!parse_cell(input + 10, &sr_sc, rows, cols)) {
                status = 0;  // Invalid input
            } else {
                status = 2;  // Valid command
            }
        }
        
        // Handle invalid input or display commands
        if (status == 0 || status == 2) {
            display_sheet(&sheet, rows, cols, toggle_display, (sr_sc >> 16), (sr_sc & 0xFFFF));
            end = get_time();
            time_taken = end - start;
            
            if (status == 0) {
                printf("[%.1f] (Invalid Input) > ", time_taken);
            } else {
                printf("[%.1f] (ok) > ", time_taken);
            }
            
            continue;
        }
        
        // Handle cell modification commands
        short_int op_row = parsed.target[0];
        short_int op_col = parsed.target[1];
        
        // Save previous cell state
        ParsedInput previous_parsed = sheet[op_row][op_col].parsed;
        sheet[op_row][op_col].parsed = parsed;

        // Update the cell and check for cycles
        short_int success = change(sheet, op_row, op_col, previous_parsed);
        
        if (!success) {
            display_sheet(&sheet, rows, cols, toggle_display, (sr_sc >> 16), (sr_sc & 0xFFFF));
            end = get_time();
            time_taken = end - start;
            printf("[%.1f] (Cycle Detected) > ", time_taken);
            continue;
        }
        
        // Display updated sheet
        display_sheet(&sheet, rows, cols, toggle_display, (sr_sc >> 16), (sr_sc & 0xFFFF));

        end = get_time();
        time_taken = end - start;
        printf("[%.1f] (ok) > ", time_taken);
    }

    // Clean up allocated memory
    free_sheet(sheet, rows, cols);

    return 0;
}