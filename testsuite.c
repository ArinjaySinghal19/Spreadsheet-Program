#include <stdlib.h>  // Required for system()
#include <stdio.h>
#include <assert.h>
#include "graph_checker.h"


void test_is_valid_cell(){
    assert(is_valid_cell("A1") == 1);
    assert(is_valid_cell("AA10") == 1);
    assert(is_valid_cell("A") == 0);
    assert(is_valid_cell("1") == 0);
    assert(is_valid_cell("A1A") == 0);
    assert(is_valid_cell("A1A1") == 0);
    assert(is_valid_cell("1A1") == 0);
}


void test_parse_cell(){
    int value;
    assert(parse_cell("A1", &value, 10, 10) == 1 && value == (0 << 16) + 0);
    assert(parse_cell("AA10", &value, 30, 30) == 1 && value == (9 << 16) + 26);
    assert(parse_cell("A", &value, 10, 10) == 0);
    assert(parse_cell("1", &value, 10, 10) == 0);
    assert(parse_cell("A1A", &value, 10, 10) == 0);
    assert(parse_cell("A1A1", &value, 10, 10) == 0);
    assert(parse_cell("1A1", &value, 10, 10) == 0);
    assert(parse_cell("A11", &value, 10, 10) == 0);
}


void test_string_to_nat(){
    assert(string_to_nat("123") == 123);
    assert(string_to_nat("0") == -1);
    assert(string_to_nat("18278") == 18278);
    assert(string_to_nat("18279") == -1);
    assert(string_to_nat("a") == -1);
    assert(string_to_nat("123a") == -1);
}

void test_process_input() {
    // Initialize test spreadsheet
    cell **sheet = malloc(10 * sizeof(cell *));
    for (int i = 0; i < 10; i++) {
        sheet[i] = malloc(10 * sizeof(cell));
        for (int j = 0; j < 10; j++) {
            sheet[i][j].value = 0;
        }
    }
    
    // Prepare test data
    ParsedInput parsed;
    initialize_parsed_input(&parsed);
    
    // Test simple value assignment (type 0)
    parsed.expression_type = '0';
    parsed.target[0] = 0;
    parsed.target[1] = 0;
    parsed.content.value_data.value = 42;
    parsed.is_value_1 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[0][0].value == 42);
    
    // Test cell reference assignment (type 0)
    parsed.expression_type = '0';
    parsed.target[0] = 1;
    parsed.target[1] = 1;
    parsed.content.value_data.value = (0 << 16) + 0;  // Reference to A1
    parsed.is_value_1 = 0;
    
    process_input(&parsed, &sheet);
    assert(sheet[1][1].value == 42);
    
    // Test binary expression (type 1, addition)
    parsed.expression_type = '1';
    parsed.target[0] = 2;
    parsed.target[1] = 2;
    parsed.operator = '+';
    parsed.content.expression_data.expression_cell[0] = 10;
    parsed.content.expression_data.expression_cell[1] = 15;
    parsed.is_value_1 = 1;
    parsed.is_value_2 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[2][2].value == 25);
    
    // Test binary expression (type 1, subtraction)
    parsed.expression_type = '1';
    parsed.target[0] = 3;
    parsed.target[1] = 3;
    parsed.operator = '-';
    parsed.content.expression_data.expression_cell[0] = 20;
    parsed.content.expression_data.expression_cell[1] = 8;
    parsed.is_value_1 = 1;
    parsed.is_value_2 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[3][3].value == 12);
    
    // Test binary expression (type 1, multiplication)
    parsed.expression_type = '1';
    parsed.target[0] = 4;
    parsed.target[1] = 4;
    parsed.operator = '*';
    parsed.content.expression_data.expression_cell[0] = 6;
    parsed.content.expression_data.expression_cell[1] = 7;
    parsed.is_value_1 = 1;
    parsed.is_value_2 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[4][4].value == 42);
    
    // Test binary expression (type 1, division)
    parsed.expression_type = '1';
    parsed.target[0] = 5;
    parsed.target[1] = 5;
    parsed.operator = '/';
    parsed.content.expression_data.expression_cell[0] = 100;
    parsed.content.expression_data.expression_cell[1] = 25;
    parsed.is_value_1 = 1;
    parsed.is_value_2 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[5][5].value == 4);
    
    // Test division by zero (should set cell to INT32_MIN)
    parsed.expression_type = '1';
    parsed.target[0] = 6;
    parsed.target[1] = 6;
    parsed.operator = '/';
    parsed.content.expression_data.expression_cell[0] = 100;
    parsed.content.expression_data.expression_cell[1] = 0;
    parsed.is_value_1 = 1;
    parsed.is_value_2 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[6][6].value == INT32_MIN);
    
    // Test binary expression with cell references
    sheet[0][1].value = 50;  // A2 = 50
    sheet[0][2].value = 10;  // A3 = 10
    
    parsed.expression_type = '1';
    parsed.target[0] = 7;
    parsed.target[1] = 7;
    parsed.operator = '+';
    parsed.content.expression_data.expression_cell[0] = (0 << 16) + 1;  // Reference to A2
    parsed.content.expression_data.expression_cell[1] = (0 << 16) + 2;  // Reference to A3
    parsed.is_value_1 = 0;
    parsed.is_value_2 = 0;
    
    process_input(&parsed, &sheet);
    assert(sheet[7][7].value == 60);
    
    // Test error propagation in binary expression
    sheet[1][2].value = INT32_MIN;  // B3 = ERROR
    
    parsed.expression_type = '1';
    parsed.target[0] = 8;
    parsed.target[1] = 8;
    parsed.operator = '+';
    parsed.content.expression_data.expression_cell[0] = 10;
    parsed.content.expression_data.expression_cell[1] = (1 << 16) + 2;  // Reference to B3
    parsed.is_value_1 = 1;
    parsed.is_value_2 = 0;
    
    process_input(&parsed, &sheet);
    assert(sheet[8][8].value == INT32_MIN);
    
    // Setup for range function tests
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            sheet[i][j].value = (i * 3) + j + 1;  // 1, 2, 3, 4, 5, 6, 7, 8, 9
        }
    }
    
    // Test MIN function (type 2)
    parsed.expression_type = '2';
    parsed.target[0] = 3;
    parsed.target[1] = 0;
    parsed.operator = '0';  // MIN
    parsed.content.function_data.function_range[0] = (0 << 16) + 0;  // A1
    parsed.content.function_data.function_range[1] = (2 << 16) + 2;  // C3
    
    process_input(&parsed, &sheet);
    assert(sheet[3][0].value == 1);
    
    // Test MAX function (type 2)
    parsed.expression_type = '2';
    parsed.target[0] = 3;
    parsed.target[1] = 1;
    parsed.operator = '1';  // MAX
    parsed.content.function_data.function_range[0] = (0 << 16) + 0;  // A1
    parsed.content.function_data.function_range[1] = (2 << 16) + 2;  // C3
    
    process_input(&parsed, &sheet);
    assert(sheet[3][1].value == 9);
    
    // Test SUM function (type 2)
    parsed.expression_type = '2';
    parsed.target[0] = 3;
    parsed.target[1] = 2;
    parsed.operator = '3';  // SUM
    parsed.content.function_data.function_range[0] = (0 << 16) + 0;  // A1
    parsed.content.function_data.function_range[1] = (2 << 16) + 2;  // C3
    
    process_input(&parsed, &sheet);
    assert(sheet[3][2].value == 45);  // Sum of 1 through 9
    
    // Test AVG function (type 2)
    parsed.expression_type = '2';
    parsed.target[0] = 3;
    parsed.target[1] = 3;
    parsed.operator = '2';  // AVG
    parsed.content.function_data.function_range[0] = (0 << 16) + 0;  // A1
    parsed.content.function_data.function_range[1] = (2 << 16) + 2;  // C3
    
    process_input(&parsed, &sheet);
    assert(sheet[3][3].value == 5);  // Average of 1 through 9
    
    // Test STDEV function (type 2)
    parsed.expression_type = '2';
    parsed.target[0] = 3;
    parsed.target[1] = 4;
    parsed.operator = '4';  // STDEV
    parsed.content.function_data.function_range[0] = (0 << 16) + 0;  // A1
    parsed.content.function_data.function_range[1] = (2 << 16) + 2;  // C3
    
    process_input(&parsed, &sheet);
    assert(sheet[3][4].value == 3);  // Standard deviation of 1 through 9 (rounded)
    
    // Test error propagation in range functions
    sheet[1][1].value = INT32_MIN;  // B2 = ERROR
    
    parsed.expression_type = '2';
    parsed.target[0] = 3;
    parsed.target[1] = 5;
    parsed.operator = '3';  // SUM
    parsed.content.function_data.function_range[0] = (0 << 16) + 0;  // A1
    parsed.content.function_data.function_range[1] = (2 << 16) + 2;  // C3
    
    process_input(&parsed, &sheet);
    assert(sheet[3][5].value == INT32_MIN);  // Error propagation
    
    // Test sleep function (type 3)
    parsed.expression_type = '3';
    parsed.target[0] = 4;
    parsed.target[1] = 0;
    parsed.content.sleep_data.sleep_value = 0;  // No sleep
    parsed.is_value_1 = 1;
    
    process_input(&parsed, &sheet);
    assert(sheet[4][0].value == 0);
    
    // Test sleep with reference to another cell
    sheet[0][3].value = 0;  // A4 = 0 (no sleep)
    
    parsed.expression_type = '3';
    parsed.target[0] = 4;
    parsed.target[1] = 1;
    parsed.content.sleep_data.sleep_value = (0 << 16) + 3;  // Reference to A4
    parsed.is_value_1 = 0;
    
    process_input(&parsed, &sheet);
    assert(sheet[4][1].value == 0);
    
    // Test invalid expression type
    parsed.expression_type = '4';  // Invalid type
    parsed.target[0] = 9;
    parsed.target[1] = 9;
    sheet[9][9].value = 100;  // Set initial value
    
    process_input(&parsed, &sheet);
    assert(sheet[9][9].value == 100);  // Value should remain unchanged
    
    // Free allocated memory
    for (int i = 0; i < 10; i++) {
        free(sheet[i]);
    }
    free(sheet);
}


void test_stack(){
    stack_top *st = initialize_stack();
    stack_push(st, 1, 1);
    short_int row, col;
    top(st, &row, &col);
    assert(row == 1 && col == 1);
    stack_pop(st, NULL);
    top(st, &row, &col);
    assert(row == -1 && col == -1);
    stack_pop(st, NULL);
    top(st, &row, &col);
    assert(row == -1 && col == -1);
    stack_push(st, 2, 2);
    top(st, &row, &col);
    assert(row == 2 && col == 2);
    stack_pop(st, NULL);
    top(st, &row, &col);
    assert(row == -1 && col == -1);
    stack_pop(st, NULL);
    top(st, &row, &col);
    assert(row == -1 && col == -1);
}


void test_parse_value(){
    int value;
    assert(parse_value("A1", &value, 10, 10) == 2 && value == (0 << 16) + 0);
    assert(parse_value("AA10", &value, 30, 30) == 2 && value == (9 << 16) + 26);
    assert(parse_value("A", &value, 10, 10) == 0);
    assert(parse_value("A1A", &value, 10, 10) == 0);
    assert(parse_value("A1A1", &value, 10, 10) == 0);
    assert(parse_value("1A1", &value, 10, 10) == 0);
    assert(parse_value("A11", &value, 10, 10) == 0);
    assert(parse_value("123", &value, 10, 10) == 1 && value == 123);
    assert(parse_value("0", &value, 10, 10) == 1 && value == 0);
    assert(parse_value("18279", &value, 10, 10) == 1 && value == 18279);
}



void test_evaluate_range(){
    ParsedInput parsed;
    initialize_parsed_input(&parsed);
    assert(evaluate_range(&parsed, "(A1:B2)", 10, 10) == 1);
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(evaluate_range(&parsed, "(A1:B2", 10, 10) == 0);
    assert(evaluate_range(&parsed, "A1:B2)", 10, 10) == 0);
    assert(evaluate_range(&parsed, "A1:B2", 10, 10) == 0);
    assert(evaluate_range(&parsed, "(A1:B2", 10, 10) == 0);
    assert(evaluate_range(&parsed, "(A1:B2)", 1, 1) == 0);
    assert(evaluate_range(&parsed, "(A1:B2)", 10, 10) == 1);
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
}



void test_handle_expression(){
    ParsedInput parsed;
    initialize_parsed_input(&parsed);
    assert(handle_expression(&parsed, "MIN(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '0');
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "MAX(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '1');
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "AVG(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '2');
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "SUM(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '3');
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "MIN(A1:B2", 10, 10) == 0);
    assert(handle_expression(&parsed, "MINA1:B2)", 10, 10) == 0);
    assert(handle_expression(&parsed, "MINA1:B2", 10, 10) == 0);
    assert(handle_expression(&parsed, "MIN(A1:B2", 10, 10) == 0);
    assert(handle_expression(&parsed, "MIN(A1:B2)", 1, 1) == 0);
    assert(handle_expression(&parsed, "MIN(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '0'); 
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "MAX(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '1');
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "AVG(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '2');
    assert(parsed.content.function_data.function_range[0] == (0 << 16) + 0);
    assert(parsed.content.function_data.function_range[1] == (1 << 16) + 1);
    assert(handle_expression(&parsed, "SUM(A1:B2)", 10, 10) == 1);
    assert(parsed.expression_type == '2');
    assert(parsed.operator == '3');
}

void test_add_dependency() {
    // Initialize test spreadsheet
    cell **sheet = malloc(5 * sizeof(cell *));
    for (int i = 0; i < 5; i++) {
        sheet[i] = malloc(5 * sizeof(cell));
        for (int j = 0; j < 5; j++) {
            sheet[i][j].value = 0;
            sheet[i][j].dependencies = NULL;
        }
    }
    
    // Test adding a new dependency
    add_dependency(sheet, 0, 0, 1, 1); // A1 depends on B2
    assert(sheet[0][0].dependencies != NULL);
    assert(sheet[0][0].dependencies->row == 1);
    assert(sheet[0][0].dependencies->col == 1);
    assert(sheet[0][0].dependencies->next == NULL);
    
    // Test adding a second dependency
    add_dependency(sheet, 0, 0, 2, 2); // A1 depends on C3
    assert(sheet[0][0].dependencies->next != NULL);
    assert(sheet[0][0].dependencies->next->row == 2);
    assert(sheet[0][0].dependencies->next->col == 2);
    
    // Test adding a duplicate dependency (shouldn't add)
    Node *last = sheet[0][0].dependencies->next;
    add_dependency(sheet, 0, 0, 1, 1); // A1 depends on B2 (already added)
    assert(sheet[0][0].dependencies->next == last); // Should still be the same node
    
    // Free allocated memory
    Node *current = sheet[0][0].dependencies;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    
    for (int i = 0; i < 5; i++) {
        free(sheet[i]);
    }
    free(sheet);
}

void test_free_from_list() {
    // Create a test dependency list
    Node *head = NULL;
    
    // Add some nodes
    Node *node1 = malloc(sizeof(Node));
    node1->row = 1;
    node1->col = 1;
    node1->next = NULL;
    head = node1;
    
    Node *node2 = malloc(sizeof(Node));
    node2->row = 2;
    node2->col = 2;
    node2->next = NULL;
    node1->next = node2;
    
    Node *node3 = malloc(sizeof(Node));
    node3->row = 3;
    node3->col = 3;
    node3->next = NULL;
    node2->next = node3;
    
    // Test removing from the middle
    head = free_from_list(head, 2, 2);
    assert(head->row == 1);
    assert(head->col == 1);
    assert(head->next->row == 3);
    assert(head->next->col == 3);
    assert(head->next->next == NULL);
    
    // Test removing the head
    head = free_from_list(head, 1, 1);
    assert(head->row == 3);
    assert(head->col == 3);
    assert(head->next == NULL);
    
    // Test removing the only element
    head = free_from_list(head, 3, 3);
    assert(head == NULL);
    
    // Test removing from empty list
    head = free_from_list(head, 4, 4);
    assert(head == NULL);
}

void test_free_parents() {
    // Initialize test spreadsheet
    cell **sheet = malloc(5 * sizeof(cell *));
    for (int i = 0; i < 5; i++) {
        sheet[i] = malloc(5 * sizeof(cell));
        for (int j = 0; j < 5; j++) {
            sheet[i][j].value = 0;
            sheet[i][j].dependencies = NULL;
        }
    }
    
    // Setup test dependencies
    add_dependency(sheet, 1, 1, 0, 0); // B2 depends on A1
    add_dependency(sheet, 2, 2, 0, 0); // C3 depends on A1
    
    // Test value reference (type 0)
    ParsedInput previous;
    initialize_parsed_input(&previous);
    previous.expression_type = '0';
    previous.is_value_1 = 0;
    previous.content.value_data.value = (1 << 16) + 1; // Reference to B2
    
    free_parents(sheet, 0, 0, previous);
    // Check B2 no longer has A1 as dependent
    assert(sheet[1][1].dependencies == NULL);
    // C3 should still have A1 as dependent
    assert(sheet[2][2].dependencies != NULL);
    assert(sheet[2][2].dependencies->row == 0);
    assert(sheet[2][2].dependencies->col == 0);
    
    // Cleanup C3 dependencies
    Node *temp = sheet[2][2].dependencies;
    free(temp);
    sheet[2][2].dependencies = NULL;
    
    // Setup test for binary expression (type 1)
    add_dependency(sheet, 1, 1, 0, 0); // B2 depends on A1
    add_dependency(sheet, 2, 2, 0, 0); // C3 depends on A1
    
    previous.expression_type = '1';
    previous.is_value_1 = 0;
    previous.is_value_2 = 0;
    previous.content.expression_data.expression_cell[0] = (1 << 16) + 1; // Reference to B2
    previous.content.expression_data.expression_cell[1] = (2 << 16) + 2; // Reference to C3
    
    free_parents(sheet, 0, 0, previous);
    // Check both B2 and C3 no longer have A1 as dependent
    assert(sheet[1][1].dependencies == NULL);
    assert(sheet[2][2].dependencies == NULL);
    
    // Setup test for range function (type 2)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            add_dependency(sheet, i, j, 0, 0); // A1-C3 all depend on A1
        }
    }
    
    previous.expression_type = '2';
    previous.content.function_data.function_range[0] = (0 << 16) + 0; // A1
    previous.content.function_data.function_range[1] = (2 << 16) + 2; // C3
    
    free_parents(sheet, 0, 0, previous);
    // Check all cells in range no longer have A1 as dependent
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            assert(sheet[i][j].dependencies == NULL);
        }
    }
    
    // Setup test for sleep function (type 3)
    add_dependency(sheet, 1, 1, 0, 0); // B2 depends on A1
    
    previous.expression_type = '3';
    previous.is_value_1 = 0;
    previous.content.sleep_data.sleep_value = (1 << 16) + 1; // Reference to B2
    
    free_parents(sheet, 0, 0, previous);
    // Check B2 no longer has A1 as dependent
    assert(sheet[1][1].dependencies == NULL);
    
    // Free allocated memory
    for (int i = 0; i < 5; i++) {
        free(sheet[i]);
    }
    free(sheet);
}

void test_update_dependencies() {
    // Initialize test spreadsheet
    cell **sheet = malloc(5 * sizeof(cell *));
    for (int i = 0; i < 5; i++) {
        sheet[i] = malloc(5 * sizeof(cell));
        for (int j = 0; j < 5; j++) {
            sheet[i][j].value = 0;
            sheet[i][j].dependencies = NULL;
            sheet[i][j].is_dirty = false;
            sheet[i][j].is_in_stack = false;
            initialize_parsed_input(&sheet[i][j].parsed);
        }
    }
    
    // Setup initial state for A1 (value reference to B2)
    ParsedInput previous;
    initialize_parsed_input(&previous);
    previous.expression_type = '0';
    previous.is_value_1 = 0;
    previous.content.value_data.value = (1 << 16) + 1; // Reference to B2
    
    // First manually add A1 as dependent of B2 (simulate existing dependency)
    add_dependency(sheet, 1, 1, 0, 0);
    
    // Set A1's current parsed input to reference C3
    sheet[0][0].parsed.expression_type = '0';
    sheet[0][0].parsed.is_value_1 = 0;
    sheet[0][0].parsed.content.value_data.value = (2 << 16) + 2; // Reference to C3
    
    // Update dependencies
    update_dependencies(sheet, 0, 0, previous);
    
    // B2 should not have A1 as dependent anymore
    assert(sheet[1][1].dependencies == NULL);
    
    // C3 should have A1 as dependent
    assert(sheet[2][2].dependencies != NULL);
    assert(sheet[2][2].dependencies->row == 0);
    assert(sheet[2][2].dependencies->col == 0);
    
    // Change A1 to a binary expression referencing B2 and D4
    previous = sheet[0][0].parsed;
    sheet[0][0].parsed.expression_type = '1';
    sheet[0][0].parsed.is_value_1 = 0;
    sheet[0][0].parsed.is_value_2 = 0;
    sheet[0][0].parsed.content.expression_data.expression_cell[0] = (1 << 16) + 1; // Reference to B2
    sheet[0][0].parsed.content.expression_data.expression_cell[1] = (3 << 16) + 3; // Reference to D4
    
    update_dependencies(sheet, 0, 0, previous);
    
    // C3 should no longer have A1 as dependent
    assert(sheet[2][2].dependencies == NULL);
    
    // B2 and D4 should have A1 as dependent
    assert(sheet[1][1].dependencies != NULL);
    assert(sheet[1][1].dependencies->row == 0);
    assert(sheet[1][1].dependencies->col == 0);
    
    assert(sheet[3][3].dependencies != NULL);
    assert(sheet[3][3].dependencies->row == 0);
    assert(sheet[3][3].dependencies->col == 0);
    
    // Change A1 to a range function referencing A1:B2
    previous = sheet[0][0].parsed;
    sheet[0][0].parsed.expression_type = '2';
    sheet[0][0].parsed.content.function_data.function_range[0] = (0 << 16) + 0; // A1
    sheet[0][0].parsed.content.function_data.function_range[1] = (1 << 16) + 1; // B2
    
    update_dependencies(sheet, 0, 0, previous);
    
    // B2 and D4 should no longer have A1 as dependent for its previous expression
    // But now B2 should have A1 as dependent for the range function
    assert(sheet[3][3].dependencies == NULL);
    
    // A1 and B2 should have A1 as dependent (reflexive dependency)
    assert(sheet[0][0].dependencies != NULL);
    assert(sheet[0][0].dependencies->row == 0);
    assert(sheet[0][0].dependencies->col == 0);
    
    assert(sheet[1][1].dependencies != NULL);
    assert(sheet[1][1].dependencies->row == 0);
    assert(sheet[1][1].dependencies->col == 0);
    
    // Change A1 to a sleep function referencing B2
    previous = sheet[0][0].parsed;
    sheet[0][0].parsed.expression_type = '3';
    sheet[0][0].parsed.is_value_1 = 0;
    sheet[0][0].parsed.content.sleep_data.sleep_value = (1 << 16) + 1; // Reference to B2
    
    update_dependencies(sheet, 0, 0, previous);
    
    // A1 should no longer have A1 as dependent
    assert(sheet[0][0].dependencies == NULL);
    
    // B2 should have A1 as dependent
    assert(sheet[1][1].dependencies != NULL);
    assert(sheet[1][1].dependencies->row == 0);
    assert(sheet[1][1].dependencies->col == 0);
    
    // Free all dependencies
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Node *current = sheet[i][j].dependencies;
            while (current != NULL) {
                Node *next = current->next;
                free(current);
                current = next;
            }
        }
    }
    
    // Free allocated memory
    for (int i = 0; i < 5; i++) {
        free(sheet[i]);
    }
    free(sheet);
}

void test_mark_dirty_and_cycle_detection() {
    // Initialize test spreadsheet
    cell **sheet = malloc(5 * sizeof(cell *));
    for (int i = 0; i < 5; i++) {
        sheet[i] = malloc(5 * sizeof(cell));
        for (int j = 0; j < 5; j++) {
            sheet[i][j].value = 0;
            sheet[i][j].dependencies = NULL;
            sheet[i][j].is_dirty = false;
            sheet[i][j].is_in_stack = false;
        }
    }
    
    // Reset global variables
    cycle = false;
    if (dfs_topo != NULL) {
        free_dirty_array(sheet);
    }
    
    // Setup a simple dependency chain: A1 -> B2 -> C3
    add_dependency(sheet, 0, 0, 1, 1); // A1 depends on B2
    add_dependency(sheet, 1, 1, 2, 2); // B2 depends on C3
    
    // Mark A1 as dirty
    mark_dirty(sheet, 0, 0);
    
    // No cycle should be detected
    assert(cycle == false);
    
    // All cells in the chain should be marked dirty
    assert(sheet[0][0].is_dirty == true);
    assert(sheet[1][1].is_dirty == true);
    assert(sheet[2][2].is_dirty == true);
    
    // Topological order should be C3, B2, A1
    assert(dfs_topo != NULL);
    assert(dfs_topo->row == 0 && dfs_topo->col == 0); // A1
    assert(dfs_topo->next != NULL);
    assert(dfs_topo->next->row == 1 && dfs_topo->next->col == 1); // B2
    assert(dfs_topo->next->next != NULL);
    assert(dfs_topo->next->next->row == 2 && dfs_topo->next->next->col == 2); // C3
    
    // Clean up
    free_dirty_array(sheet);
    
    // Test cycle detection
    // Create a cycle: A1 -> B2 -> C3 -> A1
    add_dependency(sheet, 0, 0, 1, 1); // A1 depends on B2
    add_dependency(sheet, 1, 1, 2, 2); // B2 depends on C3
    add_dependency(sheet, 2, 2, 0, 0); // C3 depends on A1 (cycle!)
    
    // Reset global variables
    cycle = false;
    
    // Mark A1 as dirty
    mark_dirty(sheet, 0, 0);
    
    // Cycle should be detected
    assert(cycle == true);
    
    // Clean up
    free_dirty_array(sheet);
    
    // Free all dependencies
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Node *current = sheet[i][j].dependencies;
            while (current != NULL) {
                Node *next = current->next;
                free(current);
                current = next;
            }
        }
    }
    
    // Free allocated memory
    for (int i = 0; i < 5; i++) {
        free(sheet[i]);
    }
    free(sheet);
}

void test_recalculate() {
    // Initialize test spreadsheet
    cell **sheet = malloc(5 * sizeof(cell *));
    for (int i = 0; i < 5; i++) {
        sheet[i] = malloc(5 * sizeof(cell));
        for (int j = 0; j < 5; j++) {
            sheet[i][j].value = 0;
            sheet[i][j].dependencies = NULL;
            sheet[i][j].is_dirty = false;
            sheet[i][j].is_in_stack = false;
            initialize_parsed_input(&sheet[i][j].parsed);
        }
    }
    
    // Reset the global dfs_topo variable to ensure clean test state
    dfs_topo = NULL;
    
    // Setup initial values
    sheet[2][2].value = 10; // C3 = 10
    
    // Setup a dependency chain: A1 -> B2 -> C3
    // A1 = B2 + 5
    sheet[0][0].parsed.expression_type = '1';
    sheet[0][0].parsed.operator = '+';
    sheet[0][0].parsed.is_value_1 = 0;
    sheet[0][0].parsed.is_value_2 = 1;
    sheet[0][0].parsed.content.expression_data.expression_cell[0] = (1 << 16) + 1; // B2
    sheet[0][0].parsed.content.expression_data.expression_cell[1] = 5; // Store literal in expression_cell since there's no separate field
    
    // B2 = C3 * 2
    sheet[1][1].parsed.expression_type = '1';
    sheet[1][1].parsed.operator = '*';
    sheet[1][1].parsed.is_value_1 = 0;
    sheet[1][1].parsed.is_value_2 = 1;
    sheet[1][1].parsed.content.expression_data.expression_cell[0] = (2 << 16) + 2; // C3
    sheet[1][1].parsed.content.expression_data.expression_cell[1] = 2; // Store literal in expression_cell
    
    // Setup dependencies correctly (who is dependent on whom)
    add_dependency(sheet, 1, 1, 0, 0); // B2 has A1 as dependent
    add_dependency(sheet, 2, 2, 1, 1); // C3 has B2 as dependent
    
    // Create a topological order: C3, B2, A1 (process in reverse dependency order)
    Node *top_order = NULL;
    insert_to_topo(&top_order, 0, 0); // A1
    insert_to_topo(&top_order, 1, 1); // B2
    insert_to_topo(&top_order, 2, 2); // C3
    
    // Mark cells as dirty
    sheet[0][0].is_dirty = true;
    sheet[1][1].is_dirty = true;
    sheet[2][2].is_dirty = true;
    
    // Recalculate
    recalculate(sheet, top_order);
    
    // Check results - C3 should still be 10 as it has no formula
    assert(sheet[2][2].value == 10); // C3 = 10
    
    // B2 should be 20 (C3 * 2)
    
    assert(sheet[1][1].value == 20); // B2 = C3 * 2 = 10 * 2 = 20
    
    // A1 should be 25 (B2 + 5)
    assert(sheet[0][0].value == 25); // A1 = B2 + 5 = 20 + 5 = 25
    
    // Cells should no longer be dirty
    assert(sheet[0][0].is_dirty == false);
    assert(sheet[1][1].is_dirty == false);
    assert(sheet[2][2].is_dirty == false);
    
    // Free allocated memory
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Node *current = sheet[i][j].dependencies;
            while (current != NULL) {
                Node *next = current->next;
                free(current);
                current = next;
            }
        }
        free(sheet[i]);
    }
    free(sheet);
}

void test_change() {
    // Initialize test spreadsheet
    cell **sheet = malloc(5 * sizeof(cell *));
    for (int i = 0; i < 5; i++) {
        sheet[i] = malloc(5 * sizeof(cell));
        for (int j = 0; j < 5; j++) {
            sheet[i][j].value = 0;
            sheet[i][j].dependencies = NULL;
            sheet[i][j].is_dirty = false;
            sheet[i][j].is_in_stack = false;
            initialize_parsed_input(&sheet[i][j].parsed);
        }
    }
    
    // Reset global variables
    cycle = false;
    if (dfs_topo != NULL) {
        free_dirty_array(sheet);
    }
    
    // Setup initial state
    sheet[2][2].value = 10; // C3 = 10
    
    // Create a previous state for A1 (empty)
    ParsedInput previous;
    initialize_parsed_input(&previous);
    
    // Set A1 to reference B2
    sheet[0][0].parsed.expression_type = '0';
    sheet[0][0].parsed.is_value_1 = 0;
    sheet[0][0].parsed.content.value_data.value = (1 << 16) + 1; // B2
    
    // Change A1
    short_int result = change(sheet, 0, 0, previous);
    
    // No cycle should be detected
    assert(result == 1);
    assert(cycle == false);
    
    // B2 should have A1 as dependent
    assert(sheet[1][1].dependencies != NULL);
    assert(sheet[1][1].dependencies->row == 0);
    assert(sheet[1][1].dependencies->col == 0);
    
    // Now create a cycle
    // Set B2 to reference A1
    previous = sheet[1][1].parsed;
    sheet[1][1].parsed.expression_type = '0';
    sheet[1][1].parsed.is_value_1 = 0;
    sheet[1][1].parsed.content.value_data.value = (0 << 16) + 0; // A1
    
    // Try to change B2
    result = change(sheet, 1, 1, previous);
    
    // Cycle should be detected and change rejected
    assert(result == 0);
    
    // B2 should be reverted to previous state
    assert(sheet[1][1].parsed.expression_type == previous.expression_type);
    
    // Test a valid change with chain of dependencies
    // Setup: C3 = 10, B2 = C3 * 2, A1 = B2 + 5
    sheet[2][2].value = 10; // C3 = 10
    
    // Setup B2 = C3 * 2
    previous = sheet[1][1].parsed;
    sheet[1][1].parsed.expression_type = '1';
    sheet[1][1].parsed.operator = '*';
    sheet[1][1].parsed.is_value_1 = 0;
    sheet[1][1].parsed.is_value_2 = 1;
    sheet[1][1].parsed.content.expression_data.expression_cell[0] = (2 << 16) + 2; // C3
    sheet[1][1].parsed.content.expression_data.expression_cell[1] = 2;
    
    result = change(sheet, 1, 1, previous);
    assert(result == 1);
    assert(sheet[1][1].value == 20); // B2 = C3 * 2 = 10 * 2 = 20
    
    // Setup A1 = B2 + 5
    previous = sheet[0][0].parsed;
    sheet[0][0].parsed.expression_type = '1';
    sheet[0][0].parsed.operator = '+';
    sheet[0][0].parsed.is_value_1 = 0;
    sheet[0][0].parsed.is_value_2 = 1;
    sheet[0][0].parsed.content.expression_data.expression_cell[0] = (1 << 16) + 1; // B2
    sheet[0][0].parsed.content.expression_data.expression_cell[1] = 5;
    
    result = change(sheet, 0, 0, previous);
    assert(result == 1);
    assert(sheet[0][0].value == 25); // A1 = B2 + 5 = 20 + 5 = 25
    
    // Now change C3 and check if A1 and B2 are updated
    previous = sheet[2][2].parsed;
    sheet[2][2].parsed.expression_type = '0';
    sheet[2][2].parsed.is_value_1 = 1;
    sheet[2][2].parsed.content.value_data.value = 15; // C3 = 15
    
    result = change(sheet, 2, 2, previous);
    assert(result == 1);
    assert(sheet[2][2].value == 15); // C3 = 15
    assert(sheet[1][1].value == 30); // B2 = C3 * 2 = 15 * 2 = 30
    assert(sheet[0][0].value == 35); // A1 = B2 + 5 = 30 + 5 = 35
    
    // Free all dependencies
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Node *current = sheet[i][j].dependencies;
            while (current != NULL) {
                Node *next = current->next;
                free(current);
                current = next;
            }
        }
    }
    
    // Free allocated memory
    for (int i = 0; i < 5; i++) {
        free(sheet[i]);
    }
    free(sheet);
}










int main() {
    test_is_valid_cell();
    test_parse_cell();
    test_string_to_nat();
    test_stack();
    test_parse_value();
    test_evaluate_range();
    test_handle_expression();
    test_process_input();
    
    // // Add graph checker tests
    test_add_dependency();
    test_free_from_list();
    test_free_parents();
    test_update_dependencies();
    test_mark_dirty_and_cycle_detection();
    // test_recalculate();
    // test_change();

    int status = system("./testing.sh");
    return status;
}
