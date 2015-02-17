#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

void d_printf(const char* format, ...) {

#ifdef _GB_DEBUG

    va_list arguments;
    va_start(arguments, format);
    int length = vsnprintf(NULL, 0, format, arguments);
    char buffer[length + 1];
    vsprintf(buffer, format, arguments);
    va_end(arguments);
    fputs(buffer, stdout);
    fflush(stdout);

#else

    return;

#endif

}
