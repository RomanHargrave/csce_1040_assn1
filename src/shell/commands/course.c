/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Implements the commands used when managing courses
 */

#include <stdlib.h>
#include <string.h>
#include "../../util.h"
#include "../model_display.h"
#include "../../tui.h"
#include "command.h"
#include "../../grading.h"

ShellReturn Command_courseList(char* args, GradeBook* gradeBook) {

    size nCourses = gradeBook->coursesCount;
    char* table[nCourses][GradeBook_COURSE_COLUMN_COUNT];
    Table_allocStrings(nCourses, GradeBook_COURSE_COLUMN_COUNT, table, 255);

    GradeBook_courseTable(gradeBook, table);

    Table_printRows(stdout, GradeBook_COURSE_COLUMN_COUNT, nCourses, GradeBook_COURSE_TABLE_COLUMNS, table);

    Table_unallocStrings(nCourses, GradeBook_COURSE_COLUMN_COUNT, table);

    return SR_SUCCESS;
}

ShellReturn Command_course(char* args, GradeBook* gradeBook) {

    char* action    = strtok(args, " ");
    char* courseId  = strtok(NULL, " ");

    if(!action | !courseId) {
        printf("Please specify an action and courseId (show, add, rm)\n");
        return SR_FAILURE;
    }

    size idNum = strtoul(courseId, NULL, 10);

    if(idNum > BYTE_MAX) {
        printf("`id` must be a value between %u and %u inclusive\n", BYTE_MIN, BYTE_MAX);
        return SR_FAILURE;
    }

    Course* course = bsearch(&(Course){.courseId = (byte)idNum}, gradeBook->courses, gradeBook->coursesCount, sizeof(Course), &Course_compareById);

    if(!course && (strcmp(action, "add") != 0)) {
        printf("No course could be found with the courseId `%lu`\n", idNum);
        return SR_FAILURE;
    }

    if((strcmp(action, "show") == 0)) {
        size nStudents = Course_studentsCount(course);
        printf("Course «%s». %lu students. Overall average is %3.02f\n\n", course->courseName, nStudents, Course_averageGrade(course));

        char* table[nStudents][Course_STUDENT_COLUMNS_COUNT];
        Table_allocStrings(nStudents, Course_STUDENT_COLUMNS_COUNT, table, 255);
        Course_studentsTable(course, table);
        Table_printRows(stdout, Course_STUDENT_COLUMNS_COUNT, nStudents, Course_STUDENT_COLUMNS, table);
        Table_unallocStrings(nStudents, Course_STUDENT_COLUMNS_COUNT, table);
    } else if(strcmp(action, "add") == 0) {

        if(gradeBook->coursesCount >= NMEMBERS(gradeBook->courses, Course)) {
            printf("No more courses may be stored in the gradebook\n");
            return SR_FAILURE;
        }

        char nameBuffer[255];

        printf("Enter a course name: ");
        fflush(stdout);
        fgets(nameBuffer, 254, stdin);

        String_trim(nameBuffer);

        Course newCourse = {
                .courseId = (byte) idNum,
        };

        strcpy(newCourse.courseName, nameBuffer);

        GradeBook_addCourse(gradeBook, newCourse);

        printf("Course added\n");

    } else if(strcmp(action, "rm") == 0) {

        if(gradeBook->coursesCount == 0) {
            printf("There are no courses in the gradebook\n");
            return SR_FAILURE;
        }

        char response[2];
        char* courseName = Course_toString(course);
        printf("Remove %s? (y/N): ", courseName);
        free(courseName);
        fflush(stdout);
        scanf("%1s", response);

        if(strcmp(response, "y") == 0 || strcmp(response, "Y") == 0) {
            GradeBook_removeCourse(gradeBook, course);
            printf("Course removed\n");
        }

    } else {
        printf("Invalid action `%s`\n", action);
        return SR_FAILURE;
    }

    return SR_SUCCESS;
}