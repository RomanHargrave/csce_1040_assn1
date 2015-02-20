/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Fills a gradebook file with valid data.
 */

#include "../models/model_io.h"

const byte nStudents    = 100;
const byte nCourses     = 25;
const char* fileName    = "serial_gradebook.gb";

int main() {

    setbuf(stdout, NULL);

    printf("Initializing GradeBook\n");

    GradeBook   index = {};

    printf("Creating %u courses:\n", nCourses);

    for(byte idx = 0; idx < nCourses; ++idx) {

        Course course = {
                .courseId   = idx,
                .courseName = "TESTCR #%02u"
        };

        sprintf(course.courseName, course.courseName, course.courseId);

        GradeBook_addCourse(&index, course);
    }

    printf("Creating %u students:\n", nStudents);

    for(byte idx = 0; idx < nStudents; ++idx) {

        Student student = {
                .studentId      = idx,
        };

        sprintf(student.studentName, "Test Student #%02u", student.studentId);

        GradeBook_addStudent(&index, student);
    }

    printf("Associate students and courses\n");

    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        char* courseName = Course_toString(&index.courses[courseIdx]);

        printf("Associations for %s:\n\n", courseName);

        for(byte studentOffset = 0; studentOffset < (nStudents / nCourses); ++studentOffset) {
            printf("-> Adding student %u of %u\n", studentOffset + 1, (nStudents / nCourses));
            byte start = courseIdx * (nStudents / nCourses);
            Course_addStudent(&index.courses[courseIdx], &index.students[start + studentOffset]);
        }

        printf("-> Done: %s\n\n", courseName);
        free(courseName);
    }

    printf("Opening file %s\n", fileName);

    FILE* writePtr = fopen(fileName, "w");

    printf("Serializing gradebook\n");

    size gbSize = sizeOfGradeBook(&index);
    printf("%s should be %lu bytes long\n", GradeBook_toString(&index), gbSize);


    byte gbSerial[gbSize];

    switch(GradeBook_serialize(&index, gbSerial)) {
        case SUCCESS:
            printf("Writing serialized GradeBook\n");
            fwrite(gbSerial, sizeof(byte), gbSize, writePtr);
            break;
        case ILLEGAL_COURSE_ID:
        case ILLEGAL_STUDENT_ID:
            printf("Invalid reference (see output). File not written\n");
            return 1;
        case BAD_MAGIC:
        case SHORT_BUFFER:
        case FAILURE:
            printf("Other failure encountered. File not written\n");
            return 1;
    }

    /*
     * Good ol' close-n-flush!
     */
    fclose(writePtr);

    // Test Deserialization --------------------------------------------------------------------------------------------

    printf("Testing GradeBook deserialization\n");

    GradeBook anotherIndex = {};

    FILE* readPtr = fopen(fileName, "r");
    size fSize = (size) fsize(readPtr);

    byte gbReadIn[fSize];

    fread(gbReadIn, fSize, sizeof(byte), readPtr);

    printf("Read %lu bytes. Calling deserializer\n", fSize);

    switch(GradeBook_deserialize(gbReadIn, &anotherIndex)) {
        case SUCCESS:
            printf("Success!\n");
            break;
        case ILLEGAL_COURSE_ID:
        case ILLEGAL_STUDENT_ID:
            printf("Invalid reference (see output). File not written\n");
            fclose(readPtr);
            return 1;
        case BAD_MAGIC:
            printf("Invalid magic number sequence.\n");
            fclose(readPtr);
            return 1;
        case SHORT_BUFFER:
        case FAILURE:
            printf("Other failure encountered. File not written\n");
            fclose(readPtr);
            return 1;
    }

    printf("Read %s \n", GradeBook_toString(&anotherIndex));

    for(byte courseIdx = 0; courseIdx < anotherIndex.coursesCount; ++courseIdx) {
        char* courseName = Course_toString(&anotherIndex.courses[courseIdx]);
        printf("-> %s\n", courseName);
        free(courseName);
        size nStudents = Course_studentsCount(&anotherIndex.courses[courseIdx]);
        for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
            char* studentName = Student_toString(anotherIndex.courses[courseIdx].students[studentIdx]);
            printf("----> %s\n", studentName);
            free(studentName);
        }
    }

    for(byte studentIdx = 0; studentIdx < anotherIndex.studentsCount; ++ studentIdx) {
        char* studentName = Student_toString(&anotherIndex.students[studentIdx]);
        printf("-> %s\n", studentName);
        free(studentName);
        size nCourses = Student_coursesCount(&anotherIndex.students[studentIdx]);
        for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
            char* courseName = Course_toString(anotherIndex.students[studentIdx].courses[courseIdx].course);
            printf("----> %s\n", courseName);
            free(courseName);
        }
    }

    return 0;
}