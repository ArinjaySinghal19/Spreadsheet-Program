#include<stdio.h>
#include<string.h>
#include<math.h>
#include<unistd.h>
#include<stdint.h>


void process_input(ParsedInput * parsed, cell *** sheet) {
    if(parsed->expression_type == -1){
        return;
    }

    if(parsed->expression_type == 3) {
        int val = parsed->content.sleep_data.sleep_value;
        if(parsed->content.sleep_data.is_value == 0) {
            short_int row = val >> 16;
            short_int col = val & 0xFFFF;
            val = (*sheet)[row][col].value;
        }
        if(val > 0) {
            sleep(val);
        }
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }

    else if(parsed->expression_type == 0){
        int val = parsed->content.value_data.value;
        if(parsed->content.value_data.is_value == 0){
            short_int row = val >> 16;
            short_int col = val & 0xFFFF;
            val = (*sheet)[row][col].value;
        }
        (*sheet)[parsed->target[0]][parsed->target[1]].value = val;
    }

    else if(parsed->expression_type == 1){
        int val1 = parsed->content.expression_data.expression_cell[0];
        if(parsed->content.expression_data.is_value_1 == 0){
            short_int row = val1 >> 16;
            short_int col = val1 & 0xFFFF;
            val1 = (*sheet)[row][col].value;
        }
        int val2 = parsed->content.expression_data.expression_cell[1];
        if(parsed->content.expression_data.is_value_2 == 0){
            short_int row = val2 >> 16;
            short_int col = val2 & 0xFFFF;
            val2 = (*sheet)[row][col].value;
        }
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
        short_int start_row = (parsed->content.function_data.function_range[0]) >> 16;
        short_int start_col = (parsed->content.function_data.function_range[0]) & 0xFFFF;
        short_int end_row = (parsed->content.function_data.function_range[1]) >> 16;
        short_int end_col = (parsed->content.function_data.function_range[1]) & 0xFFFF;
        if(parsed->content.function_data.function_operator == 0){
            int minCellValue = (*sheet)[start_row][start_col].value;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
                    if((*sheet)[i][j].value < minCellValue){
                        minCellValue = (*sheet)[i][j].value;
                    }
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = minCellValue;
        }
        else if(parsed->content.function_data.function_operator == 1){ //max
            int max = (*sheet)[start_row][start_col].value;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
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
            int sum = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
                    sum += (*sheet)[i][j].value;
                }
            }
            int mean = sum / ((end_row - start_row + 1) * (end_col - start_col + 1));
            (*sheet)[parsed->target[0]][parsed->target[1]].value = mean;
        }else if(parsed->content.function_data.function_operator == 3){ //sum
            int sum = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
                    sum += (*sheet)[i][j].value;
                }
            }
            (*sheet)[parsed->target[0]][parsed->target[1]].value = sum;
        }else if(parsed->content.function_data.function_operator == 4){ //stdev
            int sum = 0;
            int count = 0;
            for(short_int i = start_row; i <= end_row; i++){
                for(short_int j = start_col; j <= end_col; j++){
                    if((*sheet)[i][j].value == INT32_MIN){
                        (*sheet)[parsed->target[0]][parsed->target[1]].value = INT32_MIN;
                        return;
                    }
                    sum += (*sheet)[i][j].value;
                    count++;
                }
            }
            int mean = sum / count;
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
