#include<stdio.h>
#include<string.h>
// #include "initializer.h"
// #include "input_parser.h"
#include<math.h>
#include<unistd.h>
#include<stdint.h>

void process_input(ParsedInput * parsed, cell *** sheet) {

    if(parsed->expression_type == 3) {
        int val = (parsed->content.sleep_data.sleep_value[0]!=-1 ? (*sheet)[parsed->content.sleep_data.sleep_value[0]][parsed->content.sleep_data.sleep_value[1]].value : parsed->content.sleep_data.sleep_value[1]);
        if(val > 0) {
            sleep(val);
        }
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }

    else if(parsed->expression_type == 0){
        int val = (parsed->content.value_data.value[0]!=-1 ? (*sheet)[parsed->content.value_data.value[0]][parsed->content.value_data.value[1]].value : parsed->content.value_data.value[1]);
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }

    else if(parsed->expression_type == 1){
        int val1 = (parsed->content.expression_data.expression_cell_1[0]!=-1 ? (*sheet)[parsed->content.expression_data.expression_cell_1[0]][parsed->content.expression_data.expression_cell_1[1]].value : parsed->content.expression_data.expression_cell_1[1]);
        int val2 = (parsed->content.expression_data.expression_cell_2[0]!=-1 ? (*sheet)[parsed->content.expression_data.expression_cell_2[0]][parsed->content.expression_data.expression_cell_2[1]].value : parsed->content.expression_data.expression_cell_2[1]);
        if(val1 == INT32_MIN || val2 == INT32_MIN){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
            return;
        }
        if(parsed->content.expression_data.expression_operator == '+'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 + val2;
        }else if(parsed->content.expression_data.expression_operator == '-'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 - val2;
        }else if(parsed->content.expression_data.expression_operator == '*'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 * val2;
        }else if(parsed->content.expression_data.expression_operator == '/'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = (val2 == 0 ? INT32_MIN : val1 / val2);
        }
    }
    
    else if(parsed->expression_type == 2){ //min
        int start_row = parsed->content.function_data.function_range[0];
        int start_col = parsed->content.function_data.function_range[1];
        int end_row = parsed->content.function_data.function_range[2];
        int end_col = parsed->content.function_data.function_range[3];
        if(parsed->content.function_data.function_operator == 0){
            int minCellValue = (*sheet)[start_row][start_col].value;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value < minCellValue){
                        minCellValue = (*sheet)[i][j].value;
                        if(minCellValue == INT32_MIN){
                            (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                            return;
                        }
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = minCellValue;
        }else if(parsed->content.function_data.function_operator == 1){ //max
            int max = (*sheet)[start_row][start_col].value;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value > max){
                        max = (*sheet)[i][j].value;
                        if((*sheet)[i][j].value == INT32_MIN){
                            (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                            return;
                        }
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = max;
        }
    
    else if(parsed->content.function_data.function_operator == 2){ //average
            int sum = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
                }
            }
            int mean = sum / ((end_row - start_row + 1) * (end_col - start_col + 1));
            (*sheet)[parsed->target[0]][parsed->target[1]].value = mean;
        }else if(parsed->content.function_data.function_operator == 3){ //sum
            int sum = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    if(sum == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = sum;
        }else if(parsed->content.function_data.function_operator == 4){ //stdev
            int sum = 0;
            int count = 0;
            for(int i = start_row; i <= end_row; i++){
                for(int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    count++;
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
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
