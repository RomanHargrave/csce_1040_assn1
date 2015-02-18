
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "util.h"

const byte BYTE_MIN = 0;
const byte BYTE_MAX = 255;

void String_trim(char* string) {

    size initial = strlen(string);

    if(initial <= 0) return;

    char* seek = string;

    while(isspace(*seek)) ++seek;

    memmove(string, seek, initial);

    for(size pos = strlen(string) - 1; isspace(string[pos]); --pos) {
        string[pos] = 0x00;
    }
}

/*
 * Searches (linear) for the first NULL pointer and returns the index that precedes that
 * Solves the issue of determining the length of a pointer array, as all pointers will have been allocated, but will be
 * pointing to null.
 *
 * Parameter nMembers takes the allocated number of members, not the assigned number
 */
size PtrArray_CountSane(void** array, size nMembers) {

    for(size idx = 0; idx < nMembers; ++idx) {
        if(!array[idx]) {
            return idx;
        }
    }

    return 0;
}

bool Array_Contains(const void* array, const void* subject, size nMembers, size memberSize, int (* comparator)(void const*, void const*)) {
    for(size idx = 0; idx < nMembers; ++idx) {
        if(comparator(array + (idx * memberSize), subject) == 0) {
            return true;
        }
    }
    return false;
}

long fsize(FILE* file) {

    long original = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, original, SEEK_SET);

    return size;
}
