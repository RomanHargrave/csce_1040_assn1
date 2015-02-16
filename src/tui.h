/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Provides utilities for displaying formatted and meaningful output of data
 */

#ifndef _H_MENU
    #define _H_MENU
    #include "util.h"

// Begin header "menu" -------------------------------------------------------------------------------------------------

/*
 * Returns the number of characters occupied by the largest possible value of a number that occupies n amount
 * of memory. This could also add one, in order to calculate the potential for a sign bit.
 */
size numberWidth(size numberSize, byte displayBase);

/*
 * Takes an array, number of element, and element bytewidth and iterates through each element, calling *stringifier on
 * each element. The stringifier is a function that takes a number pointer and an arbitrary value, converts the value
 * to a string, and returns the string.
 *
 * Once this process is complete, Array_toString will return the string. In order to get the string's length,
 * strlen may be called upon the return value.
 */
char* Array_toString(const void* array, size nElements, size memberSize, char* delim, char*(*stringifier)(const void*));

void Table_allocStrings(size nrows, size ncolumns, char* rows[nrows][ncolumns], size strlen);

void Table_unallocStrings(size nrows, size ncolumns, char* rows[nrows][ncolumns]);

/*
 * This function accepts five parameters, where
 * `stream` is a stream
 * `columns` contains the name for each column, and will be used to display the table header
 * `rows` contains the data for each column
 * `nColumns` describes the number of columns to extract
 * `nRows` describes the number of rows present in the data
 *
 * This function will calculate the widest string value present in each column.
 * In order to accomplish this, nested iteration of complexity O(nColumns * nRows * O(n)) is required and 'n' is the
 * length of the longest string contained in the data.
 *
 * Knowing the maximum width of each column, it will proceed to first print the header, with each column name aligned
 * to based on maximum width (+4 padding).
 * It will then print a divider, followed by nRows lines, where each line is the contents of the row, also aligned
 * in the same manner as the header.
 *
 * The complexity of the actual printing is about O((nRows + 2) * O(printf))
 *
 * Example:
 *
 * Course ID    Course Name    Students
 * ------------------------------------
 *         0      TESTCR 00           5
 *         1      TESTCR 01           2
 *         2      TESTCR 02           5
 *         3      TESTCR 03           7
 */
void Table_printRows(FILE* stream, size nColumns, size nRows, const char* columns[], const char* rows[][nColumns]);

// End header "menu" ---------------------------------------------------------------------------------------------------

#endif