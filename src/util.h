/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Utility header:
 *
 * Provides widely needed constants, macros, and function signatures
 */

#ifndef _H_UTIL
    #define _H_UTIL
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdbool.h>

// Start Header "util" -------------------------------------------------------------------------------------------------

// IO Tools ------------------------------------------------------------------------------------------------------------

/*
 * Calculate the file length.
 * If POSIX standard support is enabled (ifdef __USE_POSIX), fstat will be used.
 */
long fsize(FILE* file);

// Types ---------------------------------------------------------------------------------------------------------------

/*
 * Type that can be used where an unsigned 1-byte integer is needed
 */
typedef unsigned char byte;

extern const byte BYTE_MIN;
extern const byte BYTE_MAX;

/*
 * Numerical type to use in size computations.
 */
typedef size_t size;

// Arrays --------------------------------------------------------------------------------------------------------------

void String_trim(char* string);

/*
 * Macro that inserts the equation used to determine the number of members allocated for an array
 */
#define NMEMBERS(array, type) (sizeof(array) / sizeof(type))

/*
 * Determine the number of non-null pointers in an array up to first null pointer
 * See implementation in util.c
 */
size PtrArray_CountSane(void** array, size nMembers);

/*
 * Traverse the array in a linear fashion, comparing subject and array[N] with comparator until `0` is returned
 * by the specified comparator function
 */
bool Array_Contains(const void* array, const void* subject, size nMembers, size memberSize, int(* comparator)(const void*, const void*));

// End Header "util" ---------------------------------------------------------------------------------------------------

#endif
