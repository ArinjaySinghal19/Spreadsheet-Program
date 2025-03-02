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












int main() {
    test_is_valid_cell();
    test_parse_cell();
    test_string_to_nat();
    test_stack();
    test_parse_value();
    test_evaluate_range();
    test_handle_expression();

    int status = system("./testing.sh");
    return status;
}
