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
    #include <time.h>

#define gb_new(type) ((type*) malloc(sizeof(type)))
#define gb_new0(type) ((type*) calloc(1, sizeof(type)))
#define gb_free(pointer) {if(pointer) { free(pointer); }}

#define unless(predicate) if (!(predicate))
// Start Header "util" -------------------------------------------------------------------------------------------------

// Compiler Golf -------------------------------------------------------

/*
 * Attribute DRY macro
 * Saves a lot of typing
 */
#define gb_$(n, args...) __attribute__((n ( args )))

#define $PureFunction gb_$(pure)
#define $ConstFunction gb_$(const)

#define $Factory $PureFunction gb_$(warn_unused_result)
#define $Allocator $PureFunction gb_$(returns_nonnull) gb_$(warn_unused_result)

#ifdef _GB_DO_INLINING
    #define $InlineAggressive gb_$(flatten) gb_$(always_inline) inline
    #define $WrapperFunction gb_$(artificial) $InlineAggressive
#else
    #define $InlineAggressive
    #define $WrapperFunction

#endif

#define $NotNull(args...) gb_$(nonnull, args)

#define $Vis(vis) gb_$(visibility, #vis)
#define $Hidden $Vis(hidden)
#define $Internal $Vis(internal)
#define $Protected $Vis(protected)

#define $Optimize(n...) gb_$(optimize, n)

#define $NoProfiler gb_$(no_instrument_function)

#define $Weak gb_$(weak)
#define $AliasOf(name) gb(alias, #name)

#define $Comparator $ConstFunction gb_$(noinline)

// Time Tools -----------------------------------------

#define GB_Time(operation) ({\
    const register clock_t start = clock();\
    (operation);\
    clock() - start;\
})

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
$NotNull(1)
size PtrArray_CountSane(void** array, size nMembers);

/*
 * Traverse the array in a linear fashion, comparing subject and array[N] with comparator until `0` is returned
 * by the specified comparator function
 */
$NotNull(1, 5)
bool Array_Contains(const void* array, const void* subject, size nMembers, size memberSize, int(* comparator)(const void*, const void*));

// End Header "util" ---------------------------------------------------------------------------------------------------

#endif
