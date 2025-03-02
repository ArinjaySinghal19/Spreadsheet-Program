#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef int16_t short_int;

short_int is_valid_cell(const char *cell) {
    short_int i = 0;

    // Ensure the column part contains letters
    while (isalpha(cell[i])) i++;
    if (i == 0) return 0; // No letters present

    // Ensure the row part contains digits
    int j = 0;
    while (cell[i]) {
        if (!isdigit(cell[i])) return 0; // Invalid character in row part
        i++;
        j++;
    }
    if (j == 0) return 0; // No digits present
    return 1; // Valid cell reference
}


int main(){
    printf("%d\n", is_valid_cell("A"));

}