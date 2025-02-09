#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "input_parser.h"
#include "input_processing.h"

int dirty_cells = 0;
bool cycle = false;

Node *dirty_head = NULL;

void print_dependencies(cell **sheet, int row, int col);


void add_dependency(cell **sheet, int row, int col, int dep_row, int dep_col){
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

void free_parents(cell **sheet, int row, int col){
    Node *temp = sheet[row][col].depends_on;
    if (temp == NULL) return;
    while(temp != NULL){
        int parent_row = temp->row;
        int parent_col = temp->col;
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


void mark_dirty(cell **sheet, int row, int col){
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

Node* topological_order(cell **sheet, int init_row, int init_col){
    Node *top_order = NULL;
    top_order = (Node *)malloc(sizeof(Node));
    top_order->row = init_row;
    top_order->col = init_col;
    top_order->next = NULL;
    Node *tail = top_order;
    Node *temp = top_order;
    int count = 1;
    while(temp != NULL){
        int row = temp->row;
        int col = temp->col;
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

void print_dependencies(cell **sheet, int row, int col){
    Node *temp = sheet[row][col].dependencies;
    while(temp != NULL){
        printf("(%d, %d) ", temp->row, temp->col);
        temp = temp->next;
    }
    printf("\n");
}



void print_top_order(Node *top_order){
    Node *temp = top_order;
    while(temp != NULL){
        printf("(%d, %d) ", temp->row, temp->col);
        temp = temp->next;
    }
    printf("\n");
}

void recalculate(cell **sheet, Node *top_order){
    Node *temp = top_order;
    while(temp != NULL){
        int row = temp->row;
        int col = temp->col;
        process_input(&sheet[row][col].parsed, &sheet);
        sheet[row][col].is_dirty = false;
        sheet[row][col].dirty_parents = 0;
        temp = temp->next;
    }
    free_top_order(top_order);
}

void update_dependencies(cell **sheet, int row, int col){
    free_parents(sheet, row, col);
    ParsedInput parsed = sheet[row][col].parsed;
    if(parsed.expression_type == 0) {
        int row1 = parsed.value[0];
        int col1 = parsed.value[1];
        if(row1 != -1) add_dependency(sheet, row1, col1, row, col);
    }
    if(parsed.expression_type == 1){
        int row1 = parsed.expression_cell_1[0];
        int col1 = parsed.expression_cell_1[1];
        int row2 = parsed.expression_cell_2[0];
        int col2 = parsed.expression_cell_2[1];
        if(row1 != -1) add_dependency(sheet, row1, col1, row, col);
        if(row2 != -1) add_dependency(sheet, row2, col2, row, col);
    }
    if(parsed.expression_type == 2){
        int st_row = parsed.function_range[0];
        int st_col = parsed.function_range[1];
        int end_row = parsed.function_range[2];
        int end_col = parsed.function_range[3];
        for(int i = st_row; i<=end_row; i++){
            for(int j=st_col; j<=end_col; j++){
                add_dependency(sheet, i, j, row, col);
            }
        }
    }
}

void free_dirty_array(cell **sheet){
    Node *temp = dirty_head;
    while(temp != NULL){
        int row = temp->row;
        int col = temp->col;
        sheet[row][col].is_dirty = false;
        sheet[row][col].dirty_parents = 0;
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
    dirty_head = NULL;
}

int change(cell **sheet, int row, int col){
    cycle = false;
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
        return 0;
    }
    recalculate(sheet, top_order);

    return 1;
}


