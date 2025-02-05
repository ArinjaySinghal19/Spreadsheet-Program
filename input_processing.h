#include<stdio.h>
#include<string.h>
// #include "initializer.h"
// #include "input_parser.h"
#include<math.h>
#include<unistd.h>

// typedef struct {
//     int target[2]; // Target cell (row, col)
//     int expression_type; // 0 for value, 1 for expression, 2 for function
//     int is_sleep; // 1 if SLEEP function, 0 otherwise
//     int sleep_value[2]; // Sleep value (if is_sleep=1)
//     int value[2]; // Value (if expression_type=0)
//     int expression_cell_1[2]; // First cell in expression
//     int expression_cell_2[2]; // Second cell in expression
//     char expression_operator; // Operator in expression ( +, -, *, /)
//     char function_operator; {min: 0, max: 1, avg: 2, sum: 3, stdev: 4, sleep: 5}
//     int function_range[4]; // Function range (start row, start col, end row, end col)
// } ParsedInput;

void process_input(ParsedInput * parsed, cell *** sheet, int rows, int cols) {
    if(parsed->is_sleep) {
        sleep(parsed->sleep_value);
        int val = (parsed->sleep_value[0]!=-1 ? (*sheet)[parsed->sleep_value[0]][parsed->sleep_value[1]].value : parsed->sleep_value[1]);
        (*sheet)[parsed->target[0]][parsed->target[1]].value = parsed->sleep_value[1];
    }
    else if(parsed->expression_type == 0){
        int val = (parsed->value[0]!=-1 ? (*sheet)[parsed->value[0]][parsed->value[1]].value : parsed->value[1]);
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }else if(parsed->expression_type == 1){
        int val1 = (parsed->expression_cell_1[0]!=-1 ? (*sheet)[parsed->expression_cell_1[0]][parsed->expression_cell_1[1]].value : parsed->expression_cell_1[1]);
        int val2 = (parsed->expression_cell_2[0]!=-1 ? (*sheet)[parsed->expression_cell_2[0]][parsed->expression_cell_2[1]].value : parsed->expression_cell_2[1]);
        if(parsed->expression_operator == '+'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 + val2;
        }else if(parsed->expression_operator == '-'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 - val2;
        }else if(parsed->expression_operator == '*'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 * val2;
        }else if(parsed->expression_operator == '/'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 / val2;
        }
    }else if(parsed->expression_type == 2){ //min
        int start_row = parsed->function_range[0];
        int start_col = parsed->function_range[1];
        int end_row = parsed->function_range[2];
        int end_col = parsed->function_range[3];
        if(parsed->function_operator == 0){
            int minCellValue = (*sheet)[start_row][start_col].value;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value < minCellValue){
                        minCellValue = (*sheet)[i][j].value;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = minCellValue;
        }else if(parsed->function_operator == 1){ //max
            int max = (*sheet)[start_row][start_col].value;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value > max){
                        max = (*sheet)[i][j].value;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = max;
        }else if(parsed->function_operator == 2){ //average
            int sum = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                }
            }
            int mean = sum / ((end_row - start_row + 1) * (end_col - start_col + 1));
            (*sheet)[parsed->target[0]][parsed->target[1]].value = mean;
        }else if(parsed->function_operator == 3){ //sum
            int sum = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = sum;

        }else if(parsed->function_operator == 4){ //stdev
            int sum = 0;
            int count = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    count++;
                }
            }
            double mean = (double)sum / count;
            double stdev = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    stdev += ((*sheet)[i][j].value - mean) * ((*sheet)[i][j].value - mean);
                }
            }
            stdev = stdev / count;
            stdev = sqrt(stdev);
            (*sheet)[parsed->target[0]][parsed->target[1]].value = (int) stdev;
        }else{
            return;
        }
    }
}