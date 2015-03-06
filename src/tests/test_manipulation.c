/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Attempts to reproduce an error had apperaed during serializer design.
 */

#include <assert.h>
#include <string.h>
#include "../models/model_io.h"
#include "../shell/model_display.h"
#include "../tui.h"
#include "../debug.h"

const byte nStudents    = 18;
const byte nCourses     = 3;
const char* fileName    = "test_manip_index.gb";

SerializationStatus t_openGradeBook(char* path, GradeBook* destination) {

    FILE* fptr  = fopen(path, "r");
    long flen   = fsize(fptr);

    byte buffer[flen];
    fread(buffer, sizeof(byte), flen, fptr);
    fclose(fptr);

    return GradeBook_deserialize(buffer, destination);
}

SerializationStatus t_saveGradeBook(char* path, GradeBook* source) {

    const size flen = sizeOfGradeBook(source);
    byte buffer[flen];

    SerializationStatus stat = GradeBook_serialize(source, buffer);

    FILE* fptr  = fopen(path, "w");
    fwrite(buffer, sizeof(byte), flen, fptr);
    fclose(fptr);

    return stat;
}

int main() {

    trace_enable();

    setbuf(stdout, NULL);

    GradeBook   index = {};

    for(byte idx = 0; idx < nCourses; ++idx) {

        Course course = {
                .courseId   = idx,
                .courseName = "TESTCR #%02u"
        };

        sprintf(course.courseName, course.courseName, course.courseId);

        GradeBook_addCourse(&index, course);
    }

    for(byte idx = 0; idx < nStudents; ++idx) {

        Student student = {
                .studentId      = idx,
        };

        GradeBook_addStudent(&index, student);
    }

    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {

        for(byte studentOffset = 0; studentOffset < (nStudents / nCourses); ++studentOffset) {
            byte start = courseIdx * (nStudents / nCourses);
            Course_addStudent(&index.courses[courseIdx], &index.students[start + studentOffset]);
        }

    }

    switch(t_saveGradeBook(fileName, &index)) {
        case SUCCESS:
            break;
        default:
            return 1;
    }

    printf("\n\n\n");

    //------------------------------------------------------------------------------------------------------------------

    GradeBook anotherIndex = {};

    switch(t_openGradeBook(fileName, &anotherIndex)){
        case SUCCESS:
            break;
        default:
            return 1;
    }

    char* idStudents[anotherIndex.coursesCount][GradeBook_STUDENT_COLUMN_COUNT];

    Table_allocStrings(anotherIndex.studentsCount, GradeBook_STUDENT_COLUMN_COUNT, idStudents, 255);

    printf("\n\nPristine\n\n");

    GradeBook_studentsTable(&anotherIndex, idStudents);
    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, anotherIndex.studentsCount, GradeBook_STUDENT_TABLE_COLUMNS, idStudents);

    //

    assert(GradeBook_removeStudent(&anotherIndex, &(Student){.studentId =  9}));
    assert(GradeBook_removeStudent(&anotherIndex, &(Student){.studentId = 13}));
    assert(GradeBook_removeStudent(&anotherIndex, &(Student){.studentId = 17}));

    //

    printf("\n\nPost-remove, pre-save\n\n");

    GradeBook_studentsTable(&anotherIndex, idStudents);
    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, anotherIndex.studentsCount, GradeBook_STUDENT_TABLE_COLUMNS, idStudents);

    //

    printf("\n\nPost-remove, post-save\n\n");

    t_saveGradeBook(fileName, &anotherIndex);

    GradeBook_studentsTable(&anotherIndex, idStudents);
    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, anotherIndex.studentsCount, GradeBook_STUDENT_TABLE_COLUMNS, idStudents);


    //

    printf("\n\nPost-remove, post-load\n\n");

    t_openGradeBook(fileName, &anotherIndex);

    GradeBook_studentsTable(&anotherIndex, idStudents);
    Table_printRows(stdout, GradeBook_STUDENT_COLUMN_COUNT, anotherIndex.studentsCount, GradeBook_STUDENT_TABLE_COLUMNS, idStudents);

    Table_unallocStrings(anotherIndex.studentsCount, GradeBook_STUDENT_COLUMN_COUNT, idStudents);

    return 0;
}