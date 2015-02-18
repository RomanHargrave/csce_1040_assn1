#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "../../tui.h"
#include "../model_display.h"
#include "../../grading.h"

ShellReturn Command_studentList(char* args, GradeBook* gradeBook) {

    size nStudents = gradeBook->studentsCount;
    char* table[nStudents][GradeBook_STUDENT_COLUMN_COUNT];
    Table_allocStrings(nStudents, GradeBook_STUDENT_COLUMN_COUNT, table, 255);

    GradeBook_studentsTable(gradeBook, table);

    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, nStudents, GradeBook_STUDENT_TABLE_COLUMNS, table);


    Table_unallocStrings(nStudents, GradeBook_STUDENT_COLUMN_COUNT, table);

    return SR_SUCCESS;
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

        char* studentName = Student_toString(student);
        printf("Remove %s? (y/N): ", studentName);
        free(studentName);
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