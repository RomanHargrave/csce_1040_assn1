/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Provides d_printf, a condition printf.
 * I do not think I need to go any further.
 */

#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

#ifdef _GB_DEBUG
const bool GB_DEBUG = true;
#else
const bool GB_DEBUG = false;
#endif

void d_printf(const char* format, ...) {

    if(!GB_DEBUG) return;

    va_list arguments;

    printf("[D] ");

    va_start(arguments, format);

    vfprintf(stdout, format, arguments);

    va_end(arguments);

}
