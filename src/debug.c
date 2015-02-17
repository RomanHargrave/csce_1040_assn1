#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

#ifdef _GB_DEBUG
const static bool GB_DEBUG = true;
#else
const static bool GB_DEBUG = false;
#endif

void d_printf(const char* format, ...) {

    if(!GB_DEBUG) return;

    va_list arguments;
    va_start(arguments, format);
    int length = vsnprintf(NULL, 0, format, arguments);
    char buffer[length + 1];
    vsprintf(buffer, format, arguments);
    va_end(arguments);
    fputs(buffer, stdout);
    fflush(stdout);

}
