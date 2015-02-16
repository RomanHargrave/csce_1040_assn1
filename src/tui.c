#include "tui.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

const size COL_PADDING = 4;

size numberWidth(size numberSize, byte displayBase) {
    /*
     * When we look at a number in base N, it is safe to assume that the maximum decimal value of one place in base N is
     * equivalent to N^Place. Numbersize refers to the width of a number in terms of bytes, where each byte can hold max
     * value of 256.
     *
     * One approach to calculating the number of digits in a number is as follows:
     *
     *      NDigits[an_,b_]:=Floor[x/.Solve[x-1==Log[base,n],x][[1]]/.{base->b,n->an}]
     *
     * Where `an` is an arbitrary number, and `b` is the base in which it is to be displayed
     *
     * We can convert this to a more simplistic mathematical expression as follows:
     *
     *      NDigits[n,b] -> Floor[(Log[b]+Log[n])/Log[b]]
     *
     *      Which can be further simplified to
     *
     *      1+Floor[Log[n]/Log[b]]
     *
     *      Where Log of arity 1 is the natural logarithm
     *
     * Converting from the rather syntactically... odd... language that is the Wolfram Language, we can express this
     * as follows in C
     *
     *      #include <math.h>
     *
     *      1 + floor(log(n)/log(b))
     *
     * Where `n` is the number, and `b` is the display base.
     *
     * For the above formula, we know `b` but not `n`. However, we do know how much space `n` takes up in memory, and
     * we can therefore calculate the largest possible unsigned value for `n`.
     *
     * This can be done as follows:
     *
     *      We know that numberSize represents the number of bytes in the number.
     *      In each byte there are 2 nibbles. Each nibble is equivalent to one place.
     *      By raising the maximum value of one byte to the number of places, we can ascertain
     *      the maximum value for a number of `n` bytes.
     *
     *      NIBBLE_MAX^(numberSize * 2) - 1
     *
     *      Where `NIBBLE_MAX` is 16 on nearly all systems in the world.
     *
     *      in C, this would look like
     *
     *      pow(16,(numberSize * 2)) - 1
     *
     *
     * Given these expressions, we can then evaluate them in order to test them:
     *
     *      Where NDigits[n,b] is as stated
     *      Where a char is 1 byte,  2 nibbles, and 8 bits,
     *      Where an int is 2 bytes, 4 nibbles, and 16 bits,
     *      Where a long is 4 bytes, 8 nibbles, and 32 bits
     *
     *      NDigits[16^sizeof(char),    10] -> 3
     *      NDigits[16^sizeof(int),     10] -> 5
     *      NDigits[16^sizeof(long),    10] -> 10
     *
     */
    return 1 + (size)floor(log(pow(16, (numberSize * 2)) - 1)/log(displayBase));
}

char* Array_toString(const void* array, size nElements, size memberSize, char* delim, char* (* stringifier)(void const*)) {

    /*
     * Collection of string arrays produced by the stringifier
     * We get the size by finding the largest possible element
     */

    size lengthTotal = (nElements - 1) * (strlen(delim));
    for(size idx = 0; idx < nElements; ++idx) {
        lengthTotal += strlen(stringifier(array + (idx * memberSize)));
    }

    char* buffer = calloc(sizeof(char), lengthTotal);

    for(size idx = 0; idx < nElements; ++idx) {
        char* str = stringifier(array + (idx * memberSize));
        if(idx > 0) strcat(buffer, delim);
        strcat(buffer, str);
    }

    return buffer;

}

typedef enum E_Align {

    RIGHT   = 0x01,
    LEFT    = 0x02

} Align;


void String_alignInSpace(const char* string, size width, Align alignment, char destination[]) {

    size strLength = strlen(string) + 1;

    if(strlen(string) >= width) {
        strncpy(destination, string, width);
        return;
    }

    size shift = width - strLength;

    char filler[shift];
    memset(filler, ' ', shift);
    filler[shift] = NULL;

    sprintf(destination, "%s%s",
            (alignment == RIGHT ? filler : string), (alignment == RIGHT ? string : filler));
}

void Table_allocStrings(size nrows, size ncolumns, char* rows[nrows][ncolumns], size strlen) {
    for(size rowidx = 0; rowidx < nrows; ++rowidx){
        for(size colidx = 0; colidx < ncolumns; ++colidx) {
            rows[rowidx][colidx] = calloc(sizeof(char), strlen);
        }
    }
}

void Table_unallocStrings(size nrows, size ncolumns, char* rows[nrows][ncolumns]) {
    for(size rowidx = 0; rowidx < nrows; ++rowidx){
        for(size colidx = 0; colidx < ncolumns; ++colidx) {
            free(rows[rowidx][colidx]);
        }
    }
}

void Table_printRows(FILE* stream, size nColumns, size nRows, const char* columns[], const char* rows[][nColumns]) {

    // Calculate widest column -----------------------------------------------------------------------------------------
    size columnWidth[nColumns];

    // Populate initial column sizes
    for(size idx = 0; idx < nColumns; ++idx) {
        columnWidth[idx] = strlen(columns[idx]) + COL_PADDING;
    }
    for(size rowIdx = 0; rowIdx < nRows; ++rowIdx) {
        for(size colIdx = 0; colIdx < nColumns; ++colIdx) {
            size length = strlen(rows[rowIdx][colIdx]) + COL_PADDING;
            columnWidth[colIdx] = (length > columnWidth[colIdx]) ? length : columnWidth[colIdx];
        }
    }

    columnWidth[nColumns] -= COL_PADDING;

    size widthTotal = 0;
    for(size idx = 0; idx < nColumns; ++idx) {
        widthTotal += columnWidth[idx];
    }

    widthTotal -= COL_PADDING;

    // Format header ---------------------------------------------------------------------------------------------------

    for(size idx = 0; idx < nColumns; ++idx) {
        char colBuff[columnWidth[idx]];
        String_alignInSpace(columns[idx], columnWidth[idx], LEFT, colBuff);
        fputs(colBuff, stream);
    }

    fputs("\n", stream);

    for(size nthHyphen = 0; nthHyphen < widthTotal; ++nthHyphen) {
        fputs("-", stream);
    }

    fputs("\n", stream);

    for(size rowIdx = 0; rowIdx < nRows; ++rowIdx) {
        for(size colIdx = 0; colIdx < nColumns; ++colIdx) {
            char colBuff[columnWidth[colIdx]];
            String_alignInSpace(rows[rowIdx][colIdx], columnWidth[colIdx], LEFT, colBuff);
            fputs(colBuff, stream);
        }
        fputs("\n", stream);
    }

}
