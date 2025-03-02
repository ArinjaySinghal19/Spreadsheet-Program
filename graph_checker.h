#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "input_parser.h"
#include "input_processing.h"

// Global variables for cycle detection and topological sort
bool cycle = false;
Node* dfs_topo = NULL;

/**
 * Adds a dependency relationship between cells
 * * sheet The spreadsheet
 * * row Row of the cell being depended on
 * * col Column of the cell being depended on
 * * dep_row Row of the dependent cell
 * * dep_col Column of the dependent cell
 */
void add_dependency(cell **sheet, short_int row, short_int col, short_int dep_row, short_int dep_col) {
    Node *current = sheet[row][col].dependencies;
    Node *prev = NULL; 

    // Check if dependency already exists
    while (current != NULL) {
        if (current->row == dep_row && current->col == dep_col) {
            return;  // Dependency already exists
        }
        prev = current;
        current = current->next;
    }
    
    // Create new dependency node
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = dep_row;
    new_node->col = dep_col;
    new_node->next = NULL;

    // Add to dependency list
    if (prev == NULL) {
        sheet[row][col].dependencies = new_node;  // First dependency
    } else { 
        prev->next = new_node;  // Append to existing dependencies
    }
}

/**
 * Removes a specific dependency from a linked list
 * * head Head of the dependency list
 * * row Row of the cell to remove
 * * col Column of the cell to remove
 * * Updated head of the list
 */
Node* free_from_list(Node* head, int row, int col) {
    Node *temp = head;
    Node *prev = NULL;
    
    while (temp != NULL) {
        Node *next = temp->next;
        
        if (temp->row == row && temp->col == col) {
            // Remove this node
            if (prev == NULL) {
                head = next;  // Removing the head
            } else {
                prev->next = next;  // Remove from middle/end
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

/**
 * Removes all dependencies from the previous cell formula
 * * sheet The spreadsheet
 * * row Row of the cell being updated
 * * col Column of the cell being updated
 * * previous_parsed Previous parsed input for this cell
 */
void free_parents(cell **sheet, short_int row, short_int col, ParsedInput previous_parsed) {
    // Handle simple value reference
    if (previous_parsed.expression_type == '0') {
        if (previous_parsed.is_value_1 == 0) {
            short_int row1 = previous_parsed.content.value_data.value >> 16;
            short_int col1 = previous_parsed.content.value_data.value & 0xFFFF;
            sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);
        }
        return;
    }
    
    // Handle binary expressions (operations like +, -, *, /)
    if (previous_parsed.expression_type == '1') {
        if (previous_parsed.is_value_1 == 0) {
            short_int row1 = previous_parsed.content.expression_data.expression_cell[0] >> 16;
            short_int col1 = previous_parsed.content.expression_data.expression_cell[0] & 0xFFFF;
            sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);
        }
        if (previous_parsed.is_value_2 == 0) {
            short_int row1 = previous_parsed.content.expression_data.expression_cell[1] >> 16;
            short_int col1 = previous_parsed.content.expression_data.expression_cell[1] & 0xFFFF;
            sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);
        }
        return;
    }
    
    // Handle range functions (SUM, AVG, etc.)
    if (previous_parsed.expression_type == '2') {
        short_int st_row = previous_parsed.content.function_data.function_range[0] >> 16;
        short_int st_col = previous_parsed.content.function_data.function_range[0] & 0xFFFF;
        short_int end_row = previous_parsed.content.function_data.function_range[1] >> 16;
        short_int end_col = previous_parsed.content.function_data.function_range[1] & 0xFFFF;
        // printf("Removing dependencies from range (%d, %d) to (%d, %d)\n", st_row, st_col, end_row, end_col);
        for (short_int i = st_row; i <= end_row; i++) {
            for (short_int j = st_col; j <= end_col; j++) {
                // printf("Removing dependency from (%d, %d) to (%d, %d)\n", i, j, row, col);
                sheet[i][j].dependencies = free_from_list(sheet[i][j].dependencies, row, col);
            }
        }
        return;
    }
    
    // Handle SLEEP function
    if (previous_parsed.expression_type == '3') {
        if (previous_parsed.is_value_1 == 0) {
            short_int row1 = previous_parsed.content.sleep_data.sleep_value >> 16;
            short_int col1 = previous_parsed.content.sleep_data.sleep_value & 0xFFFF;
            sheet[row1][col1].dependencies = free_from_list(sheet[row1][col1].dependencies, row, col);
        }
        return;
    }
}

/**
 * Updates cell dependencies based on new formula
 * * sheet The spreadsheet
 * * row Row of the cell being updated
 * * col Column of the cell being updated
 * * previous_parsed Previous parsed input for this cell
 */
void update_dependencies(cell **sheet, short_int row, short_int col, ParsedInput previous_parsed) {
    // First remove all old dependencies
    free_parents(sheet, row, col, previous_parsed);
    
    // Get current parsed input
    ParsedInput parsed = sheet[row][col].parsed;
    
    // Handle simple value reference
    if (parsed.expression_type == '0') {
        if (parsed.is_value_1 == 0) {
            short_int row1 = parsed.content.value_data.value >> 16;
            short_int col1 = parsed.content.value_data.value & 0xFFFF;
            add_dependency(sheet, row1, col1, row, col);
        }
        return;
    }
    
    // Handle binary expressions (operations like +, -, *, /)
    if (parsed.expression_type == '1') {
        if (parsed.is_value_1 == 0) {
            short_int row1 = parsed.content.expression_data.expression_cell[0] >> 16;
            short_int col1 = parsed.content.expression_data.expression_cell[0] & 0xFFFF;
            add_dependency(sheet, row1, col1, row, col);
        }
        if (parsed.is_value_2 == 0) {
            short_int row1 = parsed.content.expression_data.expression_cell[1] >> 16;
            short_int col1 = parsed.content.expression_data.expression_cell[1] & 0xFFFF;
            add_dependency(sheet, row1, col1, row, col);
        }
        return;
    }
    
    // Handle range functions (SUM, AVG, etc.)
    if (parsed.expression_type == '2') {
        short_int st_row = (parsed.content.function_data.function_range[0]) >> 16;
        short_int st_col = (parsed.content.function_data.function_range[0]) & 0xFFFF;
        short_int end_row = (parsed.content.function_data.function_range[1]) >> 16;
        short_int end_col = (parsed.content.function_data.function_range[1]) & 0xFFFF;
        for (short_int i = st_row; i <= end_row; i++) {
            for (short_int j = st_col; j <= end_col; j++) {
                add_dependency(sheet, i, j, row, col);
            }
        }
        return;
    }
    
    // Handle SLEEP function
    if (parsed.expression_type == '3') {
        if (parsed.is_value_1 == 0) {
            short_int row1 = parsed.content.sleep_data.sleep_value >> 16;
            short_int col1 = parsed.content.sleep_data.sleep_value & 0xFFFF;
            add_dependency(sheet, row1, col1, row, col);
        }
        return;
    }
}

/**
 * Debug utility to print dependencies of a cell
 * * sheet The spreadsheet
 * * row Row of the cell
 * * col Column of the cell
 */
void print_dependencies(cell **sheet, short_int row, short_int col) {
    Node *temp = sheet[row][col].dependencies;
    while (temp != NULL) {
        printf("(%d, %d) ", temp->row, temp->col);
        temp = temp->next;
    }
    printf("\n");
}


/**
 * Debug utility to print topological ordering
 * * top_order Head of the topological order list
 */
void print_top_order(Node *top_order) {
    Node *temp = top_order;
    while (temp != NULL) {
        printf("(%d, %d) ", temp->row, temp->col);
        temp = temp->next;
    }
    printf("\n");
}

/**
 * Inserts a cell into the topological order list
 * * top_order Head of the topological order list
 * * row Row of the cell
 * * col Column of the cell
 */
void insert_to_topo(Node **top_order, short_int row, short_int col){
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->row = row;
    new_node->col = col;
    new_node->next = *top_order;
    *top_order = new_node;
}


/**
 * Marks a cell and its dependents as dirty (needing recalculation)
 * Also performs cycle detection
 * * sheet The spreadsheet
 * * row Row of the cell
 * * col Column of the cell
 */
void mark_dirty(cell **sheet, short_int row, short_int col) {
    stack_top *st = initialize_stack();
    stack_push(st, row, col);
    while(st->top != NULL){
        short_int row, col;
        top(st, &row, &col);
        if(!sheet[row][col].is_dirty && !sheet[row][col].is_in_stack){
            sheet[row][col].is_in_stack = true;
            Node *temp = sheet[row][col].dependencies;
            while(temp != NULL){
                if(sheet[temp->row][temp->col].is_in_stack){
                    cycle = true;
                    free_stack(st, sheet);
                    return;
                }
                if(!sheet[temp->row][temp->col].is_dirty){
                    stack_push(st, temp->row, temp->col);
                }
                temp = temp->next;
            }
        }
        else{
            stack_pop(st, sheet);
            if(sheet[row][col].is_in_stack){
                sheet[row][col].is_dirty = true;
                sheet[row][col].is_in_stack = false;
                insert_to_topo(&dfs_topo, row, col);
            }
        }
    }
    free_stack(st, sheet);
}



/**
 * Cleans up the dirty cell tracking
 * * sheet The spreadsheet
 */
void free_dirty_array(cell **sheet) {
    Node *temp = dfs_topo;
    while (temp != NULL) {
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

/**
 * Recalculates all cells in the topological order
 * * sheet The spreadsheet
 * * top_order Topological ordering of cells to recalculate
 */
void recalculate(cell **sheet, Node *top_order) {
    Node *temp = top_order;
    while (temp != NULL) {
        short_int row = temp->row;
        short_int col = temp->col;
        
        // Recalculate the cell value
        process_input(&sheet[row][col].parsed, &sheet);
        sheet[row][col].is_dirty = false;
        
        temp = temp->next;
    }
    
    // Clean up
    free_dirty_array(sheet);
}

/**
 * Handles cell content changes, updates dependencies, and recalculates affected cells
 * * sheet The spreadsheet
 * * row Row of the changed cell
 * * col Column of the changed cell
 * * previous_parsed Previous parsed input for this cell
 * * 1 if successful, 0 if a cycle was detected
 */
short_int change(cell **sheet, short_int row, short_int col, ParsedInput previous_parsed) {
    cycle = false;
    short_int old_value = sheet[row][col].value;
    
    // Update cell dependencies
    update_dependencies(sheet, row, col, previous_parsed);

    // Clean up previous calculation state if needed
    if (dfs_topo != NULL) {
        free_dirty_array(sheet);
    }

    mark_dirty(sheet, row, col);
    
    // Check for circular references
    if (cycle) {
        // Revert to previous state if cycle detected
        free_dirty_array(sheet);
        ParsedInput bad_parsed = sheet[row][col].parsed;
        sheet[row][col].parsed = previous_parsed;
        update_dependencies(sheet, row, col, bad_parsed);
        sheet[row][col].value = old_value;
        return 0;
    }

    
    // Recalculate affected cells
    recalculate(sheet, dfs_topo);
    
    return 1;
}