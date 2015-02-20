/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Implements the `index` command, which displays GradeBook contents.
 */

#include <stdio.h>
#include "command.h"
#include "../../models/models.h"

ShellReturn Command_courseList(char* args, GradeBook* gradeBook);
ShellReturn Command_studentList(char* args, GradeBook* gradeBook);

ShellReturn Command_index(char* args, GradeBook* gradeBook) {

    printf( "Courses: \n"
            "\n");

    Command_courseList(args, gradeBook);

    printf( "Students: \n"
            "\n");

    Command_studentList(args, gradeBook);

    return SR_SUCCESS;
}