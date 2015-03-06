/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Provides d_printf, a condition printf.
 * I do not think I need to go any further.
 */

#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include "debug.h"

#ifdef _GB_DEBUG

    const bool GB_DEBUG = true;

#else

    const bool GB_DEBUG = false;

#endif

// Call Tracer
// ---------------------------------------------------------------------------------------------------------------------

static bool do_trace = false;
static int trace_depth = 0;

void trace_enable() {
    do_trace = true;
}

void trace_disable() {
    do_trace = false;
}

void trace_root() {
    trace_depth = 0;
}

#ifdef _GB_DEBUG

$NoProfiler
static inline void _p_trace_inset() {

    printf("x%-4d ", trace_depth);

    register int d = 0;
    for (d; d < trace_depth && d < 25; ++d) printf(" │ ");

}

$NoProfiler
static inline char* _sym_name(void* handle) {
    Dl_info symInfo = { };
    dladdr(handle, &symInfo);

    char c[512];

    if (symInfo.dli_sname) {
        sprintf(c, "%s", symInfo.dli_sname);
    } else {
        sprintf(c, "<%p>", handle);
    }

    return c;
}

$NoProfiler
void __cyg_profile_func_enter(void* function, void* call_site) {

    if (do_trace) {
        _p_trace_inset();
        printf(" ╭→ %s\n", _sym_name(function));
    }

    ++trace_depth;
};

$NoProfiler
void __cyg_profile_func_exit(void* function, void* call_site) {
    if (trace_depth > 0) --trace_depth;

    if (do_trace) {
        _p_trace_inset();
        printf("←╯  %s\n", _sym_name(function));
    }
};


#endif

// Debugging printf
// ---------------------------------------------------------------------------------------------------------------------

$NoProfiler
void d_printf(const char* format, ...) {

    unless(GB_DEBUG) return;

    va_list arguments;

    va_start(arguments, format);

    vfprintf(stdout, format, arguments);

    va_end(arguments);

#ifdef _GB_DEBUG_URGENT
    fflush(stdout);
#endif
}

