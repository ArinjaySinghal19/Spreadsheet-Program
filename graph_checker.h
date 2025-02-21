#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "input_parser.h"
#include "input_processing.h"


bool cycle = false;
Node* dfs_topo = NULL;


void add_dependency(cell **sheet, short_int row, short_int col, short_int dep_row, short_int dep_col) {
    Node *current = sheet[row][col].dependencies;
    Node *prev = NULL; 

    while (current != NULL) {
        if (current->row == dep_row && current->col == dep_col) {
            return;
        }
        prev = current;
        current = current->next;
    }
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = dep_row;
    new_node->col = dep_col;
    new_node->next = NULL;

    if (prev == NULL) {
        sheet[row][col].dependencies = new_node;
    } else { 
        prev->next = new_node;
    }
}

Node* free_from_list(Node* head, int row, int col){
    Node *temp = head;
    Node *prev = NULL;
    while(temp != NULL){
        Node *next = temp->next;
        if(temp->row == row && temp->col == col){
            if(prev == NULL){
                head = next;
            }else{
                prev->next = next;
            }
            free(temp);
            temp = next;
            continue;
        }
        prev = temp;
        temp = next;
    }
    return head;
}

void free_parents(cell **sheet, short_int row, short_int col, ParsedInput previous_parsed){

    if(previous_parsed.expression_type == 0) {
        short_int row1 = previous_parsed.content.value_data.value[0];
        short_int col1 = previous_parsed.content.value_data.value[1];
        if(row1 != -1) sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);     
        return;
    }
    if(previous_parsed.expression_type == 1){
        short_int row1 = previous_parsed.content.expression_data.expression_cell_1[0];
        short_int col1 = previous_parsed.content.expression_data.expression_cell_1[1];
        short_int row2 = previous_parsed.content.expression_data.expression_cell_2[0];
        short_int col2 = previous_parsed.content.expression_data.expression_cell_2[1];
        if(row1 != -1) sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);
        if(row2 != -1) sheet[row2][col2].dependencies = free_from_list(sheet[row2][col2].dependencies, row, col);
        return;
    }
    if(previous_parsed.expression_type == 2){
        
        short_int st_row = previous_parsed.content.function_data.function_range[0];
        short_int st_col = previous_parsed.content.function_data.function_range[1];
        short_int end_row = previous_parsed.content.function_data.function_range[2];
        short_int end_col = previous_parsed.content.function_data.function_range[3];
        for(short_int i = st_row; i<=end_row; i++){
            for(short_int j=st_col; j<=end_col; j++){
                sheet[i][j].dependencies = free_from_list(sheet[i][j].dependencies, row, col);
            }
        }
        return;
    }
    if(previous_parsed.expression_type == 3){
        short_int row1 = previous_parsed.content.sleep_data.sleep_value[0];
        short_int col1 = previous_parsed.content.sleep_data.sleep_value[1];
        if(row1 != -1) sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);
        return;
    }

}

void update_dependencies(cell **sheet, short_int row, short_int col, ParsedInput previous_parsed){
    free_parents(sheet, row, col, previous_parsed);
    ParsedInput parsed = sheet[row][col].parsed;
    if(parsed.expression_type == 0) {
        short_int row1 = parsed.content.value_data.value[0];
        short_int col1 = parsed.content.value_data.value[1];
        if(row1 != -1) add_dependency(sheet, row1, col1, row, col);
        return;
    }
    if(parsed.expression_type == 1){
        short_int row1 = parsed.content.expression_data.expression_cell_1[0];
        short_int col1 = parsed.content.expression_data.expression_cell_1[1];
        short_int row2 = parsed.content.expression_data.expression_cell_2[0];
        short_int col2 = parsed.content.expression_data.expression_cell_2[1];
        if(row1 != -1) add_dependency(sheet, row1, col1, row, col);
        if(row2 != -1) add_dependency(sheet, row2, col2, row, col);
        return;
    }
    if(parsed.expression_type == 2){
        
        short_int st_row = parsed.content.function_data.function_range[0];
        short_int st_col = parsed.content.function_data.function_range[1];
        short_int end_row = parsed.content.function_data.function_range[2];
        short_int end_col = parsed.content.function_data.function_range[3];
        for(short_int i = st_row; i<=end_row; i++){
            for(short_int j=st_col; j<=end_col; j++){
                add_dependency(sheet, i, j, row, col);
            }
        }
        return;
    }
    if(parsed.expression_type == 3){
        short_int row1 = parsed.content.sleep_data.sleep_value[0];
        short_int col1 = parsed.content.sleep_data.sleep_value[1];
        if(row1 != -1) add_dependency(sheet, row1, col1, row, col);
        return;
    }
}

void print_dependencies(cell **sheet, short_int row, short_int col){
    Node *temp = sheet[row][col].dependencies;
    while(temp != NULL){
        printf("(%d, %d) ", temp->row, temp->col);
        temp = temp->next;
    }
    printf("\n");
}



void mark_dirty(cell **sheet, short_int row, short_int col){
    sheet[row][col].is_dirty = true;
    sheet[row][col].is_in_stack = true;
    Node *temp = sheet[row][col].dependencies;
    while(temp != NULL){
        if(sheet[temp->row][temp->col].is_in_stack){
            sheet[row][col].is_in_stack = false;
            sheet[row][col].is_dirty = false;
            cycle = true;
            return;
        }
        if(!sheet[temp->row][temp->col].is_dirty) mark_dirty(sheet, temp->row, temp->col);
        if(cycle){
            sheet[row][col].is_in_stack = false;
            sheet[row][col].is_dirty = false;
            return;
        }
        temp = temp->next;
    }
    sheet[row][col].is_in_stack = false;
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = row;
    new_node->col = col;
    new_node->next = NULL;
    if(dfs_topo == NULL){
        dfs_topo = new_node;
    }else{
        Node *temp = dfs_topo;
        new_node->next = temp;
        dfs_topo = new_node;
    }
}

Node* reverse_list(Node *head){
    Node *prev = NULL;
    Node *current = head;
    Node *next = NULL;
    while(current != NULL){
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

void free_top_order(Node *top_order){
    Node *temp = top_order;
    while(temp != NULL){
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
}

void print_top_order(Node *top_order){
    Node *temp = top_order;
    while(temp != NULL){
        printf("(%d, %d) ", temp->row, temp->col);
        temp = temp->next;
    }
    printf("\n");
}


void free_dirty_array(cell **sheet){
    Node *temp = dfs_topo;
    while(temp != NULL){
        short_int row = temp->row;
        short_int col = temp->col;
        sheet[row][col].is_dirty = false;
        sheet[row][col].is_in_stack = false;
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
    dfs_topo = NULL;
}


void recalculate(cell **sheet, Node *top_order){
    Node *temp = top_order;
    while(temp != NULL){
        short_int row = temp->row;
        short_int col = temp->col;
        process_input(&sheet[row][col].parsed, &sheet);
        sheet[row][col].is_dirty = false;
        temp = temp->next;
    }
    free_dirty_array(sheet);
}


short_int change(cell **sheet, short_int row, short_int col, ParsedInput previous_parsed){
    cycle = false;
    short_int old_value = sheet[row][col].value;
    update_dependencies(sheet, row, col, previous_parsed);
    if(dfs_topo!=NULL) free_dirty_array(sheet);
    mark_dirty(sheet, row, col);
    if(cycle){
        free_dirty_array(sheet);
        ParsedInput bad_parsed = sheet[row][col].parsed;
        sheet[row][col].parsed = previous_parsed;
        update_dependencies(sheet, row, col, bad_parsed);
        sheet[row][col].value = old_value;
        return 0;
    }
    recalculate(sheet, dfs_topo);

    return 1;
}