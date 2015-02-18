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
