#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "commands/command.h"
#include "options.h"
#include "../models/model_io.h"
#include "../tui.h"

void openGradeBook(char* path, GradeBook* destination) {

    FILE* fptr  = fopen(path, "r");
    long flen   = fsize(fptr);

    byte buffer[flen];
    fread(buffer, sizeof(byte), flen, fptr);
    fclose(fptr);

    GradeBook_deserialize(buffer, destination);
}

ShellReturn saveGradeBook(char* path, GradeBook* source) {

    const size flen = sizeOfGradeBook(source);
    byte buffer[flen];

    switch(GradeBook_serialize(source, buffer)) {
        case SUCCESS:
            break;
        case FAILURE:
        case SHORT_BUFFER:
        case BAD_MAGIC:
        case ILLEGAL_COURSE_ID:
        case ILLEGAL_STUDENT_ID:
            return SR_FAILURE;
    }

    FILE* fptr  = fopen(path, "w");
    fwrite(buffer, sizeof(byte), flen, fptr);
    fclose(fptr);

    return SR_SUCCESS;
}

const char* commandTable[][3] = {
        {"clear",       "",                                     "Clear the screen"},
        {"help",        "",                                     "Display this message"},
        {"exit",        "",                                     "Exit the application"},
        {"load",        "[path]",                               "Load the gradebook. If a path is specified, it will be loaded from there."},
        {"save",        "[path]",                               "Save the gradebook. If a path is specified, it will be saved there."},
        {"index",       "",                                     "List all courses and students in the GradeBook"},
        {"courses",     "",                                     "List all courses"},
        {"course",      "show|add|rm <id>",                     "show, add, or remove a course specified by <id>"},
        {"students",    "",                                     "List all students"},
        {"student",     "show|add|rm <id>",                     "show, add, or remove a student specified by <id>"},
        {"enroll",      "add|rm <sid> <cid>",                   "add/remove (enroll/disenroll) a student, <sid>, in a course <cid>"},
        {"grade",       "add|rm <sid> <cid> <grade|index>",     "add/remove <grade/index> for student <sid>, in course <cid>."}
};

// <Development> Make clang STFU about the args parameter being unneeded in nullary commands
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

ShellReturn Command_help(char* args, GradeBook* gradeBook) {
    printf( "GradeBook CLI Help\n"
            "Usage:\n"
            "    GradeBook > command [args]\n"
            "\n"
    );

    Table_printRows(stdout, 3, NMEMBERS(commandTable, commandTable[0]), (const char* []){"Command", "Options", "Description"}, commandTable);

    return SR_SUCCESS;
}

ShellReturn Command_clear(char* args, GradeBook* gradeBook) {

    printf("%c[2J%c[0;0H",27,27);

    return SR_SUCCESS;
}

ShellReturn Command_exit(char* args, GradeBook* gradeBook) {
    return SR_EXIT;
}

ShellReturn Command_load(char* args, GradeBook* gradeBook) {

    char* path = strtok(NULL, " ");

    if(path) {
        if(access(path, R_OK) == 0) {
            openGradeBook(path, gradeBook);
            printf("Loaded gradebook from %s\n", path);
            return SR_SUCCESS;
        } else {
            printf("Read permission denied, or file does not exist, for %s\n", path);
            return SR_FAILURE;
        }
    } else {
        return SR_LOAD;
    }
}

ShellReturn Command_save(char* args, GradeBook* gradeBook) {

    char* path = strtok(NULL, " ");

    if(path) {
        saveGradeBook(path, gradeBook);
        if(access(path, W_OK) == 0) {
            printf("Saved gradebook to %s\n", path);
            return SR_SUCCESS;
        } else {
            printf("Write permission denied for %s\n", path);
            return SR_FAILURE;
        }
    } else {
        return SR_SAVE;
    }
}

ShellReturn Command_unknown(char* args, GradeBook* gradeBook) {
    printf("Unknown command. See `help` for more information.\n");
    return SR_FAILURE;
}

#pragma clang diagnostic pop

/*
 * External commands
 */

ShellReturn Command_courseList(char* args, GradeBook* gradeBook);
ShellReturn Command_studentList(char* args, GradeBook* gradeBook);
ShellReturn Command_index(char* args, GradeBook* gradeBook);
ShellReturn Command_student(char* args, GradeBook* gradeBook);
ShellReturn Command_course(char* args, GradeBook* gradeBook);
ShellReturn Command_enroll(char* args, GradeBook* gradeBook);
ShellReturn Command_grade(char* args, GradeBook* gradeBook);

const struct A_CommandAssocation {

    char* name;

    ShellCommand command;

} commands[] = {
    {"clear",               &Command_clear},
    {"help",                &Command_help},
    {"exit",                &Command_exit},
    {"load",                &Command_load},
    {"save",                &Command_save},
    {"index",               &Command_index},
    {"students",            &Command_studentList},
    {"student",             &Command_student},
    {"courses",             &Command_courseList},
    {"course",              &Command_course},
    {"enroll",              &Command_enroll},
    {"grade",               &Command_grade}

};

ShellCommand lookupCommand(char* command) {

    if(command) {
        for (size idx = 0; idx < NMEMBERS(commands, struct A_CommandAssocation); ++idx) {
            if ((strcmp(commands[idx].name, command) == 0) && commands[idx].command) return commands[idx].command;
        }
    }

    return &Command_unknown;
}

char* str2str(const void* str){ return *(char**)str; }

int Option_runShellUI(int argCount, char** args) {

    if(argCount < 3) {
        printf("In order to start the CLI, you must specify a GradeBook location.\n");
        printf("If the file you specify does not exist already, it will be created.\n");
        return 1;
    }

    char* fileName = args[2];

    GradeBook book = {};

    if(access(fileName, F_OK|W_OK|R_OK) == 0) {
        openGradeBook(fileName, &book);
    } else if(access(fileName, W_OK|R_OK) == 0) {
        saveGradeBook(fileName, &book);
    } else {
        printf("You do not have permission to access or create the file %s\n", fileName);
        printf("Please use a different file\n");
        return 1;
    }

    do {
        fflush(stdout);

        char commandBuffer[500] = {0};

        printf("GradeBook > ");
        fflush(stdout);

        if(!fgets(commandBuffer, 499, stdin)) continue;

        String_trim(commandBuffer);

        if(strlen(commandBuffer) <= 0) continue;

        // Copy the commandBuffer, as strtok will modify char* str
        char bufferCopy[500];
        memcpy(bufferCopy, commandBuffer, 500 * sizeof(char));
        char* command = strtok(bufferCopy, " ");

        // Look up the command to invoke
        ShellCommand userAction = lookupCommand(command);

        // Run the command with the characters following the argument
        switch(userAction(commandBuffer + (strlen(command) + 1), &book)) {
            case SR_EXIT:
                printf("Goodbye!\n");
                saveGradeBook(fileName, &book);
                return 0;
            case SR_FAILURE:
                printf("The command returned an error value\n");
                printf("(!) ");
                break;
            case SR_SAVE:
                if(saveGradeBook(fileName, &book) == SR_SUCCESS) {
                    printf("Gradebook saved\n");
                } else {
                    printf("Unable to save gradebook\n");
                }
                break;
            case SR_LOAD:
                openGradeBook(fileName, &book);
                printf("Gradebook loaded\n");
                break;
            default:
                break;
        }
    } while(true);

}

int Option_runShellCmd(int argCount, char** args) {

    char* allArgs = Array_toString(args + 2, argCount - 2, sizeof(char*), ", ", &str2str);
    printf("Runing in non-interactive mode, params+2 `%s`\n", allArgs);
    free(allArgs);

    if(argCount < 4) {
        printf("In order to run a command, you must specify a GradeBook location and a command.\n");
        printf("If the file you specify does not exist already, it will be created.\n");
        return 1;
    }

    char* fileName = args[2];

    GradeBook book = {};

    if(access(fileName, F_OK|W_OK|R_OK) == 0) {
        openGradeBook(fileName, &book);
    } else if(access(fileName, W_OK|R_OK) == 0) {
        saveGradeBook(fileName, &book);
    } else {
        printf("You do not have permission to access or create the file %s\n", fileName);
        printf("Please use a different file\n");
        return 1;
    }

    char* commandText = args[3];

    char* command = strtok(commandText, " ");

    ShellCommand userCommand = lookupCommand(command);

    switch(userCommand(command, &book)){
        case SR_FAILURE:
            printf("The command returned an error value\n");
            return 1;
        default:
            break;
    }

    return 0;
}