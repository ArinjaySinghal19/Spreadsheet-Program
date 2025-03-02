#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "initializer.h"

// Global buffer for column label strings
char colStr[4];

/**
 * Converts a column number to alphabetic representation (e.g., 1->A, 27->AA)
 * param col Column number (1-indexed)
 */
void int2col(short_int col) {
    char ans[4];
    short_int i = 0;
    
    // Convert number to alphabetic representation
    while (col > 0) {
        short_int rem = col % 26;
        if (rem == 0) {
            ans[i++] = 'Z';
            col = (col / 26) - 1;
        } else {
            ans[i++] = (rem - 1) + 'A';
            col = col / 26;
        }
    }
    
    // Reverse the string (since we built it backward)
    for (short_int j = 0; j < i; j++) {
        colStr[j] = ans[i - j - 1];
    }
    colStr[i] = '\0';
    return;
}

/**
 * Displays the visible portion of the spreadsheet
 * param sheet Pointer to the spreadsheet data structure
 * param rows Total number of rows in the sheet
 * param cols Total number of columns in the sheet
 * param toggle_display Flag to enable/disable display
 * param sr Starting row (for scrolling)
 * param sc Starting column (for scrolling)
 */
void display_sheet(cell ***sheet, short_int rows, short_int cols, 
                   short_int toggle_display, short_int sr, short_int sc) {
    // Skip display if toggle_display is off
    if (toggle_display == 0) {
        return;
    }
    
    // Calculate ending row and column based on viewport size (10x10)
    short_int ec = min(sc + 10, cols);
    short_int er = min(sr + 10, rows);
    
    // Print column headers
    printf("\t");
    for (short_int j = sc; j < ec; j++) {
        int2col(j + 1);
        printf("%s\t", colStr);
    }
    printf("\n");
    
    // Print rows with data
    for (short_int i = sr; i < er; i++) {
        // Print row number
        printf("%d\t", i + 1);
        
        // Print cell values
        for (short_int j = sc; j < ec; j++) {
            if ((*sheet)[i][j].value == INT32_MIN) {
                printf("ERR\t");
            } else {
                printf("%d\t", (*sheet)[i][j].value);
            }
        }
        printf("\n");
    }
    return;
}

/**
 * Process display-related commands
 * param status Command status code (6-11)
 * param toggle_display Pointer to display toggle flag
 * param sr_sc Pointer to combined scroll position (row in high bits, col in low bits)
 * param rows Total number of rows in the sheet
 * param cols Total number of columns in the sheet
 * return 1 if successful, 0 if invalid command
 */
short_int process_display(short_int status, bool *toggle_display, int *sr_sc, 
                          short_int rows, short_int cols) {
    // Extract scroll position from combined value
    short_int sr = (*sr_sc) >> 16;
    short_int sc = (*sr_sc) & 0x0000FFFF;
    
    // Process different display commands
    if (status == 6) {
        // Scroll up
        sr = max(0, sr - 10);
    } 
    else if (status == 7) {
        // Scroll left
        sc = max(0, sc - 10);
    } 
    else if (status == 8) {
        // Scroll down
        if((sr + 10) < rows){
            sr = sr + 10;
            short_int er = min(sr + 10, rows);
            sr = max(er - 10, 0);
        }
    } 
    else if (status == 9) {
        // Scroll right
        if((sc + 10) < cols){
            sc = sc + 10;
            short_int ec = min(sc + 10, cols);
            sc = max(ec - 10, 0);
        }
    } 
    else if (status == 10) {
        // Turn display off
        *toggle_display = 0;
    } 
    else if (status == 11) {
        // Turn display on
        *toggle_display = 1;
    } 
    else {
        // Invalid display command
        return 0;
    }
    
    // Update the combined scroll position
    *sr_sc = (sr << 16) + sc;
    return 1;
}