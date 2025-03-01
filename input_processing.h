#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

/**
 * Processes input expressions and updates the spreadsheet accordingly.
 * 
 *  parsed Pointer to the parsed input structure
 *  sheet Pointer to the 2D array representing the spreadsheet
 */
void process_input(ParsedInput *parsed, cell ***sheet) {
    // Early return if expression type is invalid
    if (parsed->expression_type == '4') {
        return;
    }

    // Cell coordinates for target
    short_int target_row = parsed->target[0];
    short_int target_col = parsed->target[1];

    // Process sleep expression (type 3)
    if (parsed->expression_type == '3') {
        int val = parsed->content.sleep_data.sleep_value;
        
        // If reference to another cell, get its value
        if (parsed->is_value_1 == 0) {
            short_int row = val >> 16;
            short_int col = val & 0xFFFF;
            val = (*sheet)[row][col].value;
        }
        
        // Sleep if value is positive
        if (val > 0) {
            sleep(val);
        }
        
        // Update target cell
        (*sheet)[target_row][target_col].value = val;
    }
    // Process value assignment (type 0)
    else if (parsed->expression_type == '0') {
        int val = parsed->content.value_data.value;
        
        // If reference to another cell, get its value
        if (parsed->is_value_1 == 0) {
            short_int row = val >> 16;
            short_int col = val & 0xFFFF;
            val = (*sheet)[row][col].value;
        }
        
        // Update target cell
        (*sheet)[target_row][target_col].value = val;
    }
    // Process binary expression (type 1)
    else if (parsed->expression_type == '1') {
        int val1 = parsed->content.expression_data.expression_cell[0];
        int val2 = parsed->content.expression_data.expression_cell[1];
        char operator = parsed->operator;
        
        // If val1 is a reference, get the actual value
        if (parsed->is_value_1 == 0) {
            short_int row = val1 >> 16;
            short_int col = val1 & 0xFFFF;
            val1 = (*sheet)[row][col].value;
        }
        
        // If val2 is a reference, get the actual value
        if (parsed->is_value_2 == 0) {
            short_int row = val2 >> 16;
            short_int col = val2 & 0xFFFF;
            val2 = (*sheet)[row][col].value;
        }
        
        // Handle error values
        if (val1 == INT32_MIN || val2 == INT32_MIN) {
            (*sheet)[target_row][target_col].value = INT32_MIN;
            return;
        }
        
        // Perform the operation based on the operator
        switch (operator) {
            case '+':
                (*sheet)[target_row][target_col].value = val1 + val2;
                break;
            case '-':
                (*sheet)[target_row][target_col].value = val1 - val2;
                break;
            case '*':
                (*sheet)[target_row][target_col].value = val1 * val2;
                break;
            case '/':
                (*sheet)[target_row][target_col].value = (val2 == 0) ? INT32_MIN : val1 / val2;
                break;
        }
    }
    // Process range functions (type 2)
    else if (parsed->expression_type == '2') {
        // Extract range coordinates
        short_int start_row = (parsed->content.function_data.function_range[0]) >> 16;
        short_int start_col = (parsed->content.function_data.function_range[0]) & 0xFFFF;
        short_int end_row = (parsed->content.function_data.function_range[1]) >> 16;
        short_int end_col = (parsed->content.function_data.function_range[1]) & 0xFFFF;
        char function_type = parsed->operator;
        
        // Process MIN function
        if (function_type == '0') {
            int minCellValue = (*sheet)[start_row][start_col].value;
            
            for (short_int i = start_row; i <= end_row; i++) {
                for (short_int j = start_col; j <= end_col; j++) {
                    // Handle error values
                    if ((*sheet)[i][j].value == INT32_MIN) {
                        (*sheet)[target_row][target_col].value = INT32_MIN;
                        return;
                    }
                    
                    if ((*sheet)[i][j].value < minCellValue) {
                        minCellValue = (*sheet)[i][j].value;
                    }
                }
            }
            
            (*sheet)[target_row][target_col].value = minCellValue;
        }
        // Process MAX function
        else if (function_type == '1') {
            int maxCellValue = (*sheet)[start_row][start_col].value;
            
            for (short_int i = start_row; i <= end_row; i++) {
                for (short_int j = start_col; j <= end_col; j++) {
                    // Handle error values
                    if ((*sheet)[i][j].value == INT32_MIN) {
                        (*sheet)[target_row][target_col].value = INT32_MIN;
                        return;
                    }
                    
                    if ((*sheet)[i][j].value > maxCellValue) {
                        maxCellValue = (*sheet)[i][j].value;
                    }
                }
            }
            
            (*sheet)[target_row][target_col].value = maxCellValue;
        }
        // Process AVERAGE function
        else if (function_type == '2') {
            int sum = 0;
            int cell_count = (end_row - start_row + 1) * (end_col - start_col + 1);
            
            for (short_int i = start_row; i <= end_row; i++) {
                for (short_int j = start_col; j <= end_col; j++) {
                    // Handle error values
                    if ((*sheet)[i][j].value == INT32_MIN) {
                        (*sheet)[target_row][target_col].value = INT32_MIN;
                        return;
                    }
                    
                    sum += (*sheet)[i][j].value;
                }
            }
            
            int mean = sum / cell_count;
            (*sheet)[target_row][target_col].value = mean;
        }
        // Process SUM function
        else if (function_type == '3') {
            int sum = 0;
            
            for (short_int i = start_row; i <= end_row; i++) {
                for (short_int j = start_col; j <= end_col; j++) {
                    // Handle error values
                    if ((*sheet)[i][j].value == INT32_MIN) {
                        (*sheet)[target_row][target_col].value = INT32_MIN;
                        return;
                    }
                    
                    sum += (*sheet)[i][j].value;
                }
            }
            
            (*sheet)[target_row][target_col].value = sum;
        }
        // Process STDEV function
        else if (function_type == '4') {
            int sum = 0;
            int count = 0;
            
            // First pass: calculate sum and count
            for (short_int i = start_row; i <= end_row; i++) {
                for (short_int j = start_col; j <= end_col; j++) {
                    // Handle error values
                    if ((*sheet)[i][j].value == INT32_MIN) {
                        (*sheet)[target_row][target_col].value = INT32_MIN;
                        return;
                    }
                    
                    sum += (*sheet)[i][j].value;
                    count++;
                }
            }
            
            int mean = sum / count;
            double variance = 0.0;
            
            // Second pass: calculate variance
            for (short_int i = start_row; i <= end_row; i++) {
                for (short_int j = start_col; j <= end_col; j++) {
                    int diff = (*sheet)[i][j].value - mean;
                    variance += diff * diff;
                }
            }
            
            variance = variance / count;
            double stdev = round(sqrt(variance));
            
            (*sheet)[target_row][target_col].value = (int)stdev;
        }
        else {
            // Unknown function type
            return;
        }
    }
}