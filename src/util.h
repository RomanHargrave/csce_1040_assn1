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

#define gb_new(type) ((type*) malloc(sizeof(type)))
#define gb_new0(type) ((type*) calloc(1, sizeof(type)))
#define gb_free(pointer) {if(pointer) { free(pointer); }}

#define unless(predicate) if (!(predicate))
// Start Header "util" -------------------------------------------------------------------------------------------------

// Compiler Stuff

#define C_ATR(n) __attribute__((n))
#define C_ATR_A(n, args...) __attribute__((n ( args )))

#define F_PURE C_ATR(pure)
#define F_CONST C_ATR(const)

#define F_CREATE F_PURE C_ATR(warn_unused_result)
#define F_CONSTRUCTOR F_PURE C_ATR(returns_nonnull) C_ATR(warn_unused_result)

#define AGG_INLINE C_ATR(flatten) inline
#define F_WRAPPER C_ATR(artificial) AGG_INLINE
#define F_BRIDGE C_ATR(always_inline) F_WRAPPER

#define ALL_ARGS_EXIST C_ATR(nonnull)
#define ARGS_EXIST(args...) __attribute__((nonnull (args)))

#define F_VISIBILITY(vis) __attribute__((visibility ( #vis )))
#define F_HIDDEN F_VISIBILITY(hidden)
#define F_INTERNAL F_VISIBILITY(internal)
#define F_PROTECTED F_VISIBILITY(protected)

#define F_COMPARATOR F_CONST C_ATR(noinline)

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
ARGS_EXIST(1)
size PtrArray_CountSane(void** array, size nMembers);

/*
 * Traverse the array in a linear fashion, comparing subject and array[N] with comparator until `0` is returned
 * by the specified comparator function
 */
ARGS_EXIST(1, 5)
bool Array_Contains(const void* array, const void* subject, size nMembers, size memberSize, int(* comparator)(const void*, const void*));

// End Header "util" ---------------------------------------------------------------------------------------------------

#endif
