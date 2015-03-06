#include "util.h"/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 *
 */

#ifndef _H_DEBUG
#define _H_DEBUG
    #include <stdio.h>
    #include <stdarg.h>
    #include <stdbool.h>
    #include "util.h"

extern const bool GB_DEBUG;

void d_printf(const char* format, ...);

$NoProfiler void trace_enable();
$NoProfiler void trace_disable();
$NoProfiler void trace_root();

#endif