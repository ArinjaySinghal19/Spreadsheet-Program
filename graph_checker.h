#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "input_parser.h"
#include "input_processing.h"


short_int dirty_cells = 0;
bool cycle = false;

Node *dirty_head = NULL;


void add_dependency(cell **sheet, short_int row, short_int col, short_int dep_row, short_int dep_col){
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = dep_row;
    new_node->col = dep_col;
    new_node->next = NULL;
    if(sheet[row][col].dependencies == NULL){
        sheet[row][col].dependencies = new_node;
    }else{
        Node *temp = sheet[row][col].dependencies;
        new_node->next = temp;
        sheet[row][col].dependencies = new_node;
    }
    Node *new_node_dep = (Node *)malloc(sizeof(Node));
    new_node_dep->row = row;
    new_node_dep->col = col;
    new_node_dep->next = NULL;
    if(sheet[dep_row][dep_col].depends_on == NULL){
        sheet[dep_row][dep_col].depends_on = new_node_dep;
    }else{
        Node *temp = sheet[dep_row][dep_col].depends_on;
        new_node_dep->next = temp;
        sheet[dep_row][dep_col].depends_on = new_node_dep;
    }
}

void free_parents(cell **sheet, short_int row, short_int col){
    Node *temp = sheet[row][col].depends_on;
    if (temp == NULL) return;
    while(temp != NULL){
        short_int parent_row = temp->row;
        short_int parent_col = temp->col;
        Node *temp_dep = sheet[parent_row][parent_col].dependencies;
        Node *prev = NULL;
        while(temp_dep != NULL){
            Node *next_dep = temp_dep->next;
            if(temp_dep->row == row && temp_dep->col == col){
            if(prev == NULL){
                sheet[parent_row][parent_col].dependencies = next_dep;
            }else{
                prev->next = next_dep;
            }
            free(temp_dep);
            temp_dep = next_dep;
            continue;   
            }
            prev = temp_dep;
            temp_dep = next_dep;
        }
        temp = temp->next;
    }
    temp = sheet[row][col].depends_on;
    while(temp != NULL){
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
    sheet[row][col].depends_on = NULL;
}

void update_dependencies(cell **sheet, short_int row, short_int col){
    free_parents(sheet, row, col);
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
    dirty_cells++;
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = row;
    new_node->col = col;
    new_node->next = dirty_head;
    dirty_head = new_node;
    Node *temp = sheet[row][col].dependencies;
    while(temp != NULL){
        sheet[temp->row][temp->col].dirty_parents++;
        if (sheet[temp->row][temp->col].is_dirty == false){
            mark_dirty(sheet, temp->row, temp->col);
        }
        temp = temp->next;
    }
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

Node* topological_order(cell **sheet, short_int init_row, short_int init_col){
    Node *top_order = NULL;
    top_order = (Node *)malloc(sizeof(Node));
    top_order->row = init_row;
    top_order->col = init_col;
    top_order->next = NULL;
    Node *tail = top_order;
    Node *temp = top_order;
    short_int count = 1;
    while(temp != NULL){
        short_int row = temp->row;
        short_int col = temp->col;
        Node *temp_dep = sheet[row][col].dependencies;
        while(temp_dep != NULL){
            sheet[temp_dep->row][temp_dep->col].dirty_parents--;
            if(sheet[temp_dep->row][temp_dep->col].dirty_parents == 0){
                Node *new_node = (Node *)malloc(sizeof(Node));
                new_node->row = temp_dep->row;
                new_node->col = temp_dep->col;
                new_node->next = NULL;
                tail->next = new_node;
                tail = new_node;
                count++;
            }
            temp_dep = temp_dep->next;
        }
        temp = temp->next;
    }
    // print_top_order(top_order);
    if(count == dirty_cells){
        dirty_cells = 0;
        return top_order;
    }else{
        dirty_cells = 0;
        if(top_order!=NULL) free_top_order(top_order);
        cycle = true;
        return NULL;
    }
}



void free_dirty_array(cell **sheet){
    Node *temp = dirty_head;
    while(temp != NULL){
        short_int row = temp->row;
        short_int col = temp->col;
        sheet[row][col].is_dirty = false;
        sheet[row][col].dirty_parents = 0;
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
    dirty_head = NULL;
}


void recalculate(cell **sheet, Node *top_order){
    Node *temp = top_order;
    while(temp != NULL){
        short_int row = temp->row;
        short_int col = temp->col;
        process_input(&sheet[row][col].parsed, &sheet);
        sheet[row][col].is_dirty = false;
        sheet[row][col].dirty_parents = 0;
        temp = temp->next;
    }
    free_top_order(top_order);
}


short_int change(cell **sheet, short_int row, short_int col){
    cycle = false;
    Node *old_depends_on = sheet[row][col].depends_on;
    short_int old_value = sheet[row][col].value;
    update_dependencies(sheet, row, col);
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->row = row;
    temp->col = col;
    temp->next = NULL;
    if(dirty_head!=NULL) free_dirty_array(sheet);
    dirty_head = temp;
    mark_dirty(sheet, row, col);
    Node *top_order = topological_order(sheet, row, col);
    if(dirty_head!=NULL) free_dirty_array(sheet);
    if(cycle){
        free_top_order(top_order);
        free_parents(sheet, row, col);
        sheet[row][col].depends_on = old_depends_on;
        sheet[row][col].value = old_value;
        return 0;
    }
    recalculate(sheet, top_order);

    return 1;
}


