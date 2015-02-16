#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "options.h"
#include "../models/model_io.h"
#include "../tui.h"
#include "model_display.h"
#include "../grading.h"

typedef enum E_ShellReturn {

    SR_SUCCESS  = 0x0,
    SR_FAILURE  = 0x1,
    SR_EXIT     = 0x2,
    SR_SAVE     = 0x3,
    SR_LOAD     = 0x4

} ShellReturn;

typedef ShellReturn(*ShellCommand)(char*, GradeBook* gradeBook);

void openGradeBook(char* path, GradeBook* destination) {

    FILE* fptr  = fopen(path, "r");
    long flen   = fsize(fptr);

    byte buffer[flen];
    fread(buffer, sizeof(byte), flen, fptr);
    fclose(fptr);

    GradeBook_deserialize(buffer, destination);
}

void saveGradeBook(char* path, GradeBook* source) {

    const size flen = sizeOfGradeBook(source);
    byte buffer[flen];

    GradeBook_serialize(source, buffer);

    FILE* fptr  = fopen(path, "w");
    fwrite(buffer, sizeof(byte), flen, fptr);
    fclose(fptr);
}

const char* commandTable[][3] = {
        {"help",        "",                 "Display this message"},
        {"exit",        "",                 "Exit the application"},
        {"load",        "[path]",           "Load the gradebook. If a path is specified, it will be loaded from there."},
        {"save",        "[path]",           "Save the gradebook. If a path is specified, it will be saved there."},
        {"index",       "",                 "List all courses and students in the GradeBook"},
        {"courses",     "",                 "List all courses"},
        {"students",    "",                 "List all students"},
        {"student",     "show|add|rm <id>", "show, add, or remove a student specified by <id>"}
};

ShellReturn Command_help(char* args, GradeBook* gradeBook) {
    printf( "GradeBook CLI Help\n"
            "Usage:\n"
            "    GradeBook > command [args]\n"
            "\n"
    );

    Table_printRows(stdout, 3, NMEMBERS(commandTable, commandTable[0]), (const char* []){"Command", "Options", "Description"}, commandTable);


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

ShellReturn Command_courseList(char* args, GradeBook* gradeBook) {

    size nCourses = gradeBook->coursesCount;
    char* table[nCourses][GradeBook_COURSE_COLUMN_COUNT];
    for(size idx = 0; idx < nCourses; ++idx){
        table[idx][0]   = calloc(sizeof(char), 255);
        table[idx][1]   = calloc(sizeof(char), 255);
        table[idx][2]   = calloc(sizeof(char), 255);
        table[idx][3]   = calloc(sizeof(char), 255);
    }

    GradeBook_courseTable(gradeBook, table);

    Table_printRows(stdout, GradeBook_COURSE_COLUMN_COUNT, nCourses, GradeBook_COURSE_TABLE_COLUMNS, table);

    for(size idx = 0; idx < nCourses; ++idx){
        free(table[idx][0]);
        free(table[idx][1]);
        free(table[idx][2]);
        free(table[idx][3]);
    }

    return SR_SUCCESS;
}

ShellReturn Command_studentList(char* args, GradeBook* gradeBook) {

    size nStudents = gradeBook->studentsCount;
    char* table[nStudents][GradeBook_STUDENT_COLUMN_COUNT];
    for(size idx = 0; idx < nStudents; ++idx){
        table[idx][0]   = calloc(sizeof(char), 255);
        table[idx][1]   = calloc(sizeof(char), 255);
        table[idx][2]   = calloc(sizeof(char), 255);
        table[idx][3]   = calloc(sizeof(char), 255);
    }

    GradeBook_studentsTable(gradeBook, table);

    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, nStudents, GradeBook_STUDENT_TABLE_COLUMNS, table);

    for(size idx = 0; idx < nStudents; ++idx){
        free(table[idx][0]);
        free(table[idx][1]);
        free(table[idx][2]);
        free(table[idx][3]);
    }

    return SR_SUCCESS;
}

ShellReturn Command_index(char* args, GradeBook* gradeBook) {

    printf( "Courses: \n"
            "+-----------+\n");

    Command_courseList(args, gradeBook);

    printf( "Students: \n"
            "+-----------+\n");

    Command_studentList(args, gradeBook);

    return SR_SUCCESS;
}

ShellReturn Command_unknown(char* args, GradeBook* gradeBook) {
    printf("Unknown command. See `help` for more information.\n");
    return SR_FAILURE;
}

ShellReturn Command_student(char* args, GradeBook* gradeBook) {

    char* action    = strtok(NULL, " ");
    char* studentId = strtok(NULL, " ");

    if(!action | !studentId) {
        printf("Please specify an action and studentId (show, add, rm)\n");
        return SR_FAILURE;
    }

    size idNum = strtoul(studentId, NULL, 10);

    if(idNum > BYTE_MAX) {
        printf("`id` must be a value between %u and %u inclusive\n", BYTE_MIN, BYTE_MAX);
        return SR_FAILURE;
    }

    Student* student = bsearch(&(Student){.studentId = (byte)idNum}, gradeBook->students, gradeBook->studentsCount, sizeof(Student), &Student_compareById);

    if(!student && (strcmp(action, "add") != 0)) {
        printf("No student could be found with the studentId `%lu`\n", idNum);
        return SR_FAILURE;
    }

    if((strcmp(action, "show") == 0)) {
        size nCourses = Student_coursesCount(student);
        printf("Student «%s». %lu courses. Overall average is %3.02f\n\n", student->studentName, Student_coursesCount(student), Student_averageGrade(student));

        char* table[nCourses][Student_COURSE_COLUMNS_COUNT];
        Table_allocStrings(nCourses, Student_COURSE_COLUMNS_COUNT, table, 255);
        Student_coursesTable(student, table);
        Table_printRows(stdout, Student_COURSE_COLUMNS_COUNT, nCourses, Student_COURSE_COLUMNS, table);
        Table_unallocStrings(nCourses, Student_COURSE_COLUMNS_COUNT, table);
    } else if(strcmp(action, "add") == 0) {

        if(gradeBook->studentsCount >= NMEMBERS(gradeBook->students, student)) {
            printf("No more students may be stored in the gradebook\n");
            return SR_FAILURE;
        }

        char nameBuffer[255];

        printf("Enter a student name: ");
        fflush(stdout);
        fgets(nameBuffer, 254, stdin);

        String_trim(nameBuffer);

        Student newStudent = {
                .studentId = (byte) idNum,
        };

        strcpy(newStudent.studentName, nameBuffer);

        GradeBook_addStudent(gradeBook, newStudent);

        printf("Student added\n");
    } else if(strcmp(action, "rm") == 0) {

        if(gradeBook->studentsCount == 0) {
            printf("There are no students in the gradebook\n");
            return SR_FAILURE;
        }

        char response[2];

        printf("Remove %s? (y/N): ", Student_toString(student));
        fflush(stdout);
        scanf("%1s", response);

        if(strcmp(response, "y") == 0 || strcmp(response, "Y") == 0) {
            GradeBook_removeStudent(gradeBook, student);
            printf("Student removed\n");
        }

    } else {
        printf("Invalid action `%s`\n", action);
        return SR_FAILURE;
    }

    return SR_SUCCESS;
}

const struct A_CommandAssocation {

    char* name;

    ShellCommand command;

} commands[] = {

    {"help",                &Command_help},
    {"exit",                &Command_exit},
    {"load",                &Command_load},
    {"save",                &Command_save},
    {"index",               &Command_index},
    {"students",            &Command_studentList},
    {"student",             &Command_student},
    {"courses",             &Command_courseList},
    {"course_new",          NULL},
    {"course_rm",           NULL},
    {"course_show",         NULL},
    {"student_addCourse",   NULL},
    {"student_rmCourse",    NULL},
    {"student_addGrade",    NULL},
    {"student_delGrade",    NULL}

};

ShellCommand lookupCommand(char* command) {

    if(command) {
        for (size idx = 0; idx < NMEMBERS(commands, struct A_CommandAssocation); ++idx) {
            if ((strcmp(commands[idx].name, command) == 0) && commands[idx].command) return commands[idx].command;
        }
    }

    return &Command_unknown;
}

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
                saveGradeBook(fileName, &book);
                printf("Gradebook saved\n");
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

char* str2str(const void* str){ return *(char**)str;}

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