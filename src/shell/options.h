/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Prototypes for command-line behaviours
 */

#ifndef _H_RUN_OPTIONS
    #define _H_RUN_OPTIONS

// Begin header "run options" ------------------------------------------------------------------------------------------

typedef int(*RuntimeOption)(int, char**);

int Option_runShellUI(int argCount, char** args);

int Option_runShellCmd(int argCount, char** args);

int Option_printGradeBook(int argCount, char** args);

// End header "run options" --------------------------------------------------------------------------------------------

#endif