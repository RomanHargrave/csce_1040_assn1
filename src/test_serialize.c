#include "models/model_io.h"

const byte nStudents    = 18;
const byte nCourses     = 3;
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

        printf("Associations for %s:\n\n", Course_toString(&index.courses[courseIdx]));

        for(byte studentOffset = 0; studentOffset < (nStudents / nCourses); ++studentOffset) {
            printf("-> Adding student %u of %u\n", studentOffset + 1, (nStudents / nCourses));
            byte start = courseIdx * (nStudents / nCourses);
            index.courses[courseIdx].students[studentOffset] = &index.students[start + studentOffset];
        }

        size nCourseStudents = Course_studentsCount(&index.courses[courseIdx]);

        for(byte studentIdx = 0; studentIdx < nCourseStudents; ++studentIdx) {
            printf("-> Adding course to student %u of %lu\n", studentIdx + 1, nCourseStudents);
            Student* student = index.courses[courseIdx].students[studentIdx];
            Student_addCourse(student, &index.courses[courseIdx]);
        }

        printf("-> Done: %s\n\n", Course_toString(&index.courses[courseIdx]));
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
        printf("-> %s\n", Course_toString(&anotherIndex.courses[courseIdx]));
        size nStudents = Course_studentsCount(&anotherIndex.courses[courseIdx]);
        for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
            printf("----> %s\n", Student_toString(anotherIndex.courses[courseIdx].students[studentIdx]));
        }
    }

    for(byte studentIdx = 0; studentIdx < anotherIndex.studentsCount; ++ studentIdx) {
        printf("-> %s\n", Student_toString(&anotherIndex.students[studentIdx]));
        size nCourses = Student_coursesCount(&anotherIndex.students[studentIdx]);
        for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
            printf("----> %s\n", Course_toString(anotherIndex.students[studentIdx].courses[courseIdx].course));
        }
    }

    return 0;
}