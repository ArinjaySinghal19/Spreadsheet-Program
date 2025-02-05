#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

int dirty_cells = 0;    // global variable to keep track of dirty cells

typedef struct Node{
    int row;
    int col;
    struct Node *next;
} Node;


typedef struct cell{
    int value;
    int row;
    int col;
    Node *dependencies;
    int num_dependencies;
    int parent_row;
    int parent_col;
    bool is_dirty;
    int dirty_parents;
} cell;


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
    sheet[dep_row][dep_col].parent_row = row;
    sheet[dep_row][dep_col].parent_col = col;
}

void free_parents(cell **sheet, int row, int col){
    int par_row = sheet[row][col].parent_row;
    int par_col = sheet[row][col].parent_col;
    Node *temp = sheet[par_row][par_col].dependencies;
    Node *prev = NULL;
    while(temp != NULL){
        if(temp->row == row && temp->col == col){
            if(prev == NULL){
                sheet[par_row][par_col].dependencies = temp->next;
            }else{
                prev->next = temp->next;
            }
            free(temp);
            break;
        }
        prev = temp;
        temp = temp->next;
    }
    sheet[row][col].parent_row = -1;
    sheet[row][col].parent_col = -1;
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
    sheet[temp->row][temp->col].is_dirty = false;
    sheet[temp->row][temp->col].dirty_parents = 0;
    temp = temp->next;
    while(temp != NULL){
        int row = temp->row;
        int col = temp->col;
        int parent_row = sheet[row][col].parent_row;
        int parent_col = sheet[row][col].parent_col;
        int value = sheet[parent_row][parent_col].value;
        sheet[row][col].value = value;
        sheet[row][col].is_dirty = false;
        sheet[row][col].dirty_parents = 0;
        temp = temp->next;
    }
    free_top_order(top_order);
    dirty_cells = 0;
}

void free_dependencies(Node *dependencies){
    Node *temp = dependencies;
    while(temp != NULL){
        Node *next = temp->next;
        free(temp);
        temp = next;
    }
}

void free_sheet(cell **sheet, int row, int col){
    for(int i=0; i<row; i++){
        for(int j=0; j<col; j++){
            free_dependencies(sheet[i][j].dependencies);
        }
        free(sheet[i]);
    }
    free(sheet);
}




void display(cell **sheet){
    for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
            printf("%d ", sheet[i][j].value);
        }
        printf("\n");
    }
}

void assign_value(cell **sheet, int row, int col, int value){
    sheet[row][col].value = value;
    mark_dirty(sheet, row, col);
    Node *top_order = topological_order(sheet, row, col);
    if(top_order == NULL){
        printf("Cycle detected\n");
        return;
    }
    recalculate(sheet, top_order);
    display(sheet);
}


int main(){

    cell **sheet = (cell **)malloc(10 * sizeof(cell *));
    for(int i=0; i<10; i++){
        sheet[i] = (cell *)malloc(10 * sizeof(cell));
    }
    for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
            sheet[i][j].value = 0;
            sheet[i][j].row = i;
            sheet[i][j].col = j;
            sheet[i][j].dependencies = NULL;
            sheet[i][j].num_dependencies = 0;
            sheet[i][j].parent_row = -1;
            sheet[i][j].parent_col = -1;
            sheet[i][j].is_dirty = false;
            sheet[i][j].dirty_parents = 0;
        }
    }

    add_dependency(sheet, 0, 0, 1, 1);
    add_dependency(sheet, 0, 0, 2, 2);
    add_dependency(sheet, 0, 0, 3, 3);
    add_dependency(sheet, 0, 0, 4, 4);


    assign_value(sheet, 0, 0, 5);
    print_dependencies(sheet, 0, 0);
    printf("\n");
    free_parents(sheet, 2, 2);
    assign_value(sheet, 2, 2, 10);
    print_dependencies(sheet, 0, 0);
    printf("\n");
    add_dependency(sheet, 2, 2, 0, 0);
    assign_value(sheet, 2, 2, 15);
    printf("\n");


}
