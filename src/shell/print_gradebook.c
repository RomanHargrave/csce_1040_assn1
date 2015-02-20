/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Implements the GradeBook `dump` function used to list all models in a GradeBook file
 */

#include <stdio.h>
#include "options.h"
#include "../models/models.h"
#include "../models/model_io.h"
#include "model_display.h"
#include "../tui.h"

int Option_printGradeBook(int argCount, char** args) {

    if(argCount < 3) {
        printf("No gradebook file specified\n");
        return 1;
    }

    const char* gradeBookPath = args[2];

    FILE* stream = fopen(gradeBookPath, "r");
    long fileSize = fsize(stream);
    byte fileBuffer[fileSize];
    fread(fileBuffer, (size) fileSize, sizeof(byte), stream);
    fclose(stream);

    GradeBook index = {};

    SerializationStatus status = GradeBook_deserialize(fileBuffer, &index);
    switch(status) {
        case SHORT_BUFFER:
            printf("The grade book file was not large enough and may be corrupt.\n");
            return 1;
        case BAD_MAGIC:
            printf("The specified file does not appear to be a GradeBook file\n");
            return 1;
        case ILLEGAL_COURSE_ID:
            printf("An illegal course ID was referenced during deserialization\n");
            return 1;
        case ILLEGAL_STUDENT_ID:
            printf("An ilelgal student ID was referenced during deserialization\n");
            return 1;
        case FAILURE:
            printf("An unspecified error occurred when deserializing. Please check your scrollback.\n");
            return 1;
        case SUCCESS:
            printf("GradeBook was loaded successfully\n\n");
            break;
    }


    // 2-dimensional arrays containing strings of arbitrary length
    char* courseTable[index.coursesCount][GradeBook_COURSE_COLUMN_COUNT];
    char* studentTable[index.studentsCount][GradeBook_STUDENT_COLUMN_COUNT];
    Table_allocStrings(index.coursesCount, GradeBook_COURSE_COLUMN_COUNT, courseTable, 255);
    Table_allocStrings(index.studentsCount, GradeBook_STUDENT_COLUMN_COUNT, studentTable, 255);

    GradeBook_courseTable(&index, courseTable);
    GradeBook_studentsTable(&index, studentTable);

    printf("Courses: \n");

    Table_printRows(stdout, GradeBook_COURSE_COLUMN_COUNT, index.coursesCount,
            GradeBook_COURSE_TABLE_COLUMNS, courseTable);

    printf("\n");

    printf("Students: \n");
    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, index.studentsCount,
            GradeBook_STUDENT_TABLE_COLUMNS, studentTable);

    Table_unallocStrings(index.coursesCount, GradeBook_COURSE_COLUMN_COUNT, courseTable);
    Table_unallocStrings(index.studentsCount, GradeBook_STUDENT_COLUMN_COUNT, studentTable);

    return 0;
}