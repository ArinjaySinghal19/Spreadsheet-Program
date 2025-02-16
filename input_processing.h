#include<stdio.h>
#include<string.h>
// #include "initializer.h"
// #include "input_parser.h"
#include<math.h>
#include<unistd.h>
#include<stdint.h>


void process_input(ParsedInput * parsed, cell *** sheet) {

    if(parsed->expression_type == 3) {
        short_int val = (parsed->content.sleep_data.sleep_value[0]!=-1 ? (*sheet)[parsed->content.sleep_data.sleep_value[0]][parsed->content.sleep_data.sleep_value[1]].value : parsed->content.sleep_data.sleep_value[1]);
        if(val > 0) {
            sleep(val);
        }
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }

    else if(parsed->expression_type == 0){
        short_int val = (parsed->content.value_data.value[0]!=-1 ? (*sheet)[parsed->content.value_data.value[0]][parsed->content.value_data.value[1]].value : parsed->content.value_data.value[1]);
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }

    else if(parsed->expression_type == 1){
        short_int val1 = (parsed->content.expression_data.expression_cell_1[0]!=-1 ? (*sheet)[parsed->content.expression_data.expression_cell_1[0]][parsed->content.expression_data.expression_cell_1[1]].value : parsed->content.expression_data.expression_cell_1[1]);
        short_int val2 = (parsed->content.expression_data.expression_cell_2[0]!=-1 ? (*sheet)[parsed->content.expression_data.expression_cell_2[0]][parsed->content.expression_data.expression_cell_2[1]].value : parsed->content.expression_data.expression_cell_2[1]);
        if(val1 == INT16_MIN || val2 == INT16_MIN){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = INT16_MIN;
            return;
        }
        if(parsed->content.expression_data.expression_operator == '+'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 + val2;
        }else if(parsed->content.expression_data.expression_operator == '-'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 - val2;
        }else if(parsed->content.expression_data.expression_operator == '*'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = val1 * val2;
        }else if(parsed->content.expression_data.expression_operator == '/'){
            (*sheet)[parsed->target[0]][parsed->target[1]].value = (val2 == 0 ? INT16_MIN : val1 / val2);
        }
    }
    
    else if(parsed->expression_type == 2){ //min
        short_int start_row = parsed->content.function_data.function_range[0];
        short_int start_col = parsed->content.function_data.function_range[1];
        short_int end_row = parsed->content.function_data.function_range[2];
        short_int end_col = parsed->content.function_data.function_range[3];
        if(parsed->content.function_data.function_operator == 0){
            short_int minCellValue = (*sheet)[start_row][start_col].value;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT16_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT16_MIN;
                        return;
                    }
                    if((*sheet)[i][j].value < minCellValue){
                        minCellValue = (*sheet)[i][j].value;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = minCellValue;
        }else if(parsed->content.function_data.function_operator == 1){ //max
            short_int max = (*sheet)[start_row][start_col].value;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT16_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT16_MIN;
                        return;
                    }
                    if((*sheet)[i][j].value > max){
                        max = (*sheet)[i][j].value;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = max;
        }
    
    else if(parsed->content.function_data.function_operator == 2){ //average
            short_int sum = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    if((*sheet)[i][j].value == INT16_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT16_MIN;
                        return;
                    }
                }
            }
            short_int mean = sum / ((end_row - start_row + 1) * (end_col - start_col + 1));
            (*sheet)[parsed->target[0]][parsed->target[1]].value = mean;
        }else if(parsed->content.function_data.function_operator == 3){ //sum
            short_int sum = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    if((*sheet)[i][j].value == INT16_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT16_MIN;
                        return;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = sum;
        }else if(parsed->content.function_data.function_operator == 4){ //stdev
            short_int sum = 0;
            short_int count = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    sum += (*sheet)[i][j].value;
                    count++;
                    if((*sheet)[i][j].value == INT16_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT16_MIN;
                        return;
                    }
                }
            }
            short_int mean = sum / count;
            double stdev = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    stdev += ((*sheet)[i][j].value - mean) * ((*sheet)[i][j].value - mean);
                }
            }
            stdev = stdev / count;
            stdev = round(sqrt(stdev));
            (*sheet)[parsed->target[0]][parsed->target[1]].value = (int) stdev;
        }else{
            return;
        }
    }
}
