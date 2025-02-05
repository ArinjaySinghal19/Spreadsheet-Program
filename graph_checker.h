#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "input_parser.h"
#include "input_processing.h"

int dirty_cells = 0;    // global variable to keep track of dirty cells




void add_dependency(cell **sheet, int row, int col, int dep_row, int dep_col){
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = dep_row;
    new_node->col = dep_col;
    new_node->next = NULL;
    if(sheet[row][col].dependencies == NULL){
        sheet[row][col].dependencies = new_node;
    }else{
        Node *temp = sheet[row][col].dependencies;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_node;
    }
    sheet[row][col].num_dependencies++;
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
    while(temp != NULL){
        int parent_row = temp->row;
        int parent_col = temp->col;
        Node *temp_dep = sheet[parent_row][parent_col].dependencies;
        Node *prev = NULL;
        while(temp_dep != NULL){
            if(temp_dep->row == row && temp_dep->col == col){
                if(prev == NULL){
                    sheet[parent_row][parent_col].dependencies = temp_dep->next;
                }else{
                    prev->next = temp_dep->next;
                }
                sheet[parent_row][parent_col].num_dependencies--;
                free(temp_dep);
                break;
            }
            prev = temp_dep;
            temp_dep = temp_dep->next;
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
        return top_order;
    }else{
        free_top_order(top_order);
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
    dirty_cells = 0;
}

void update_dependencies(cell **sheet, int row, int col){
    free_parents(sheet, row, col);
    ParsedInput parsed = sheet[row][col].parsed;
    if(parsed.expression_type == 0) return;
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
            for(int j=0; j<=end_col; j++){
                add_dependency(sheet, i, j, row, col);
            }
        }
    }
}

void change(cell **sheet, int row, int col){
    update_dependencies(sheet, row, col);
    mark_dirty(sheet, row, col);
    Node *top_order = topological_order(sheet, row, col);
    if(top_order == NULL){
        printf("Cycle detected\n");
        return;
    }
    recalculate(sheet, top_order);
}


