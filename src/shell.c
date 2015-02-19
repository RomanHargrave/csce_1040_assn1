/*
 * Prompt user interface for interacting with GradeBook
 *
 */

#include <string.h>
#include "models/models.h"
#include "models/model_io.h"
#include "tui.h"
#include "grading.h"
#include "util.h"
#include "shell/options.h"

const char* PROGRAM_HEADER =
        "+----------------------------------------------------------+\n"
        "| Roman Hargrave, ***REMOVED***                          |\n"
        "| 'GradeBook', over-engineered ('enterprise') edition      |\n"
        "| UNT Computer Science Department                          |\n"
        "| CSCE 1040 Section 001, Spring 2015                       |\n"
        "+----------------------------------------------------------+\n"
        "\n";


void stringPtr2string(const void* string, FILE* stream) {
    fputs(*((char**) string), stream);
}

int Option_help(int argCount, char** args) {

    if(argCount > 1 && (strcmp(args[1], "help") != 0)) {
        char* invalidArgs = Array_toString(args + 1, argCount - 1, sizeof(char*), " ", &stringPtr2string);
        printf("Invalid argument%s: %s\n\n", (argCount - 1) > 1 ? "s" : "",invalidArgs);
        free(invalidArgs);
    }

    printf( "Usage: %s [command] <command options...>"
            "\n\n"
            "Commands:\n"
            "    help                       - Display this message\n"
            "    interactive [filename]     - Run in shell mode, filename defaults to `gradebook.gb`\n"
            "    dump [filename]            - Display the contents of a gradebook \n"
            "    apply <command> [filename] - Open the specified gradebook, and execute the command as in interactive mode\n"
            "", args[0]);

    return 0;
}


const struct A_OptionsMapping {
    char* name;
    RuntimeOption action;
} options[] = {
        {"help",        &Option_help},
        {"interactive", &Option_runShellUI},
        {"apply",       &Option_runShellCmd},
        {"dump",        &Option_printGradeBook},
};

RuntimeOption dispatchOption(char* name) {

    if(name) {
        for (size idx = 0; idx < NMEMBERS(options, struct A_OptionsMapping); ++idx) {
            if ((strcmp(options[idx].name, name) == 0) && options[idx].action) return options[idx].action;
        }
    }

    return &Option_help;
}

int main(int argCount, char** args) {

    printf(PROGRAM_HEADER);
    fflush(stdout);

    return dispatchOption(args[1])(argCount, args);
}