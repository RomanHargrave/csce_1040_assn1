/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * `model_io` provides the business logic for exposed interfaces in the companion header.
 * These interfaces transform GradeBook data in to simplified structures that can then be
 * serialized in to a buffer for transport over a network, storage on disk, what have you.
 *
 * Additionally provided are functions to pre-compute the size of this data.
 *
 * Defines model serialization function signatures and helpers
 */

/*
 * model_io.h includes models.h
 */
#include <string.h>
#include <search.h>
#include <stdlib.h>
#include <stdio.h>
#include "../util.h"

#include "model_io.h"

// Utility Functions ---------------------------------------------------------------------------------------------------

int compare_byte(const void* a, const void* b) {
    if(!a | !b) {
        return (!a && !b) ? 0 : -1;
    } else {
        return *((byte*)a) - *((byte*)b);
    }
}

// Begin IO Utilities --------------------------------------------------------------------------------------------------

const byte GRADEBOOK_MAGIC[4] = {0x01, 0xD5, 0xC0, 0x01};


/*
 * A bit on formats.
 *
 * In the formats specified in their respective section, the following symbols, if present, hold the specified values.
 *
 * '|' denotes nothing significant, simply represents where one field ends, and another begins
 * '_' simply serves to space out bytes
 * '(...)' denotes that the enclosed data is conditionally present
 * 'B' is the space of one byte
 * '[...]' is the placeholder for a large amount of arbitrary data
 *
 * Overall file format:
 * 0x00 MAGIC;
 * 0x04 GradeBook;
 * 0x?? n Course's;
 * 0x?? n Student's;
 *
 * Where the number of courses and students is provided by the GradeBook.
 */


/*
 * Intermediary models
 */

// ---- GradeBook ------------------------------------------------------------------------------------------------------

/*
 * Serial format of GradeBook (not that it is constructed from IGradeBook):
 * Also not that the GradeBook will be placed at the beginning of the serialized data following the file magic.
 *
 * B|[course ID's]|B|[student ID's]
 * - ------------- - -------------
 * | |             | |- student ID's, one byte each
 * | |             |- number of student ID's
 * | |- course ID's, one byte each
 * |- number of course ID's
 *
 * Example:
 *
 * Courses: 5
 * - Course 0
 * - Course 1
 * - Course 2
 * - Course 3
 * - Course 4
 *
 * Students: 10
 * - Student 1
 * - ...
 *
 * 0500010203040A00010203040506070809
 *  ----------- ---------------------
 *  | | | | | | | | | | | | | | | | |- Student 9
 *  | | | | | | | | | | | | | | | |- Student 8
 *  | | | | | | | | | | | | | | |- Student 7
 *  | | | | | | | | | | | | | |- Student 6
 *  | | | | | | | | | | | | |- Student 5
 *  | | | | | | | | | | | |- Student 4
 *  | | | | | | | | | | |- Student 3
 *  | | | | | | | | | |- Student 2
 *  | | | | | | | | |- Student 1
 *  | | | | | | | |- Student 0
 *  | | | | | | |- 0x0A Students
 *  | | | | | |- Course 4
 *  | | | | |- Course 3
 *  | | | |- Course 2
 *  | | |- Course 1
 *  | |- Course 0
 *  |- 0x05 Courses
 *
 *  The reason for not simply declaring the number of students and courses and being done with it, is that in the situation that
 *  a student or course is removed, all courses and students would have to be renumbered to fit the serialization system.
 *  A simpler solution is simply do what the C++ compiler does and destroy that course or student and put a new one in its place.
 */

typedef struct S_IGradeBook {

    /*
     * Courses by Course ID
     */
    byte courses[25];

    byte coursesCount;

    /*
     * Students by Student ID
     */
    byte students[100];

    byte studentsCount;

} IGradeBook;

/*
 * Serialize book in to receiver, starting at offset, and return the next NULL pointer index
 */
size IGradeBook_serialize(IGradeBook* book, byte* receiver, size offset) {

    size idx            = offset;
    byte nCourses       = book->coursesCount;
    byte nStudents      = book->studentsCount;

    // Segment 1 - nCourses, followed by nCourses of byte courseId
    receiver[idx++]     = nCourses;

    // -- Proceed to write so many course ID's
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        receiver[idx++]  = book->courses[courseIdx];
    }

    // Segment 2 - nStudents, following by nStudents of byte studentId
    receiver[idx++]    = nStudents;

    // -- Proceed to write so many student ID's
    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        receiver[idx++]  = book->students[studentIdx];
    }

    // IDX will sit at the last written byte of data.
    return idx;
}

/*
 * Deserialize a GradeBook, and return the position of the next unrelated byte
 */
size IGradeBook_deserialize(byte* data, size offset, IGradeBook* destination) {

    size idx        = offset;
    byte nCourses   = 0;
    byte nStudents  = 0;

    // Segment 1 - Get course count
    nCourses        = data[idx++];

    // -- Read course ID's
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        destination->courses[courseIdx] = data[idx++];
    }

    // Segment 2 - Get student count
    nStudents       = data[idx++];

    // -- Read student ID's
    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        destination->students[studentIdx] = data[idx++];
    }

    // Sort course and student ID's
    qsort(destination->courses, nCourses, sizeof(byte), &compare_byte);
    qsort(destination->students, nStudents, sizeof(byte), &compare_byte);

    destination->coursesCount = nCourses;
    destination->studentsCount = nStudents;

    return idx;
}

// ---- Course ---------------------------------------------------------------------------------------------------------

/*
 * Serial format of Course (note that it is constructed from ICourse):
 *
 * B|[student ID's]|B|B|[name]
 * - -------------  - - ------
 * | |              | | |- Course name, ASCII data.
 * | |              | |- Course name length in bytes
 * | |              |- Course ID
 * | |- student ID's, one byte per
 * |- Number of students
 *
 * Example:
 *
 * Course ID 0xAA (170)
 * Course name PSYC 1500 (50 53 59 43 20 31 35 30 30)
 * Name length: 10 (0x0A)
 *
 * Students: 3
 * - 0x01
 * - 0x02
 * - 0x03
 *
 * 03010203AA0A505359432031353030
 *  ------- - -------------------
 *  |       | |- 0xA5 is name length, followed by 0xA5 bytes of ASCII encoded characters
 *  |       |- 0xAA is Course ID
 *  |- 0x03 Students
 */

typedef struct S_ICourse {

    /*
     * Students by student ID
     */
    byte students[20];

    byte studentsCount;

    /**
    * Describes the course ID
    * Once again, a range of [0, 255] which exceeds the 25 course limit by far.
    */
    byte courseId;

    /**
    * Describes the course name
    * The course name should not reasonably exceed 16 characters, if it should be in the format of a college course
    * name, such as 'CSCE 1040' (8 Characters).
    */
    char courseName[255];

} ICourse;

/*
 * GNU C Comparator for ICourse that compares by courseId
 */
int ICourse_compareByID(const void* a, const void* b) {
    if( !a | !b /* NULL Pointer check */) {
        return (!a && !b) ? 0 : -1 /* Always send NULL to the end */ ;
    } else {
        return ((ICourse *) a)->courseId - ((ICourse *) b)->courseId;
    }
}

/*
 * Initializes an ICourse (primitive course serialization intermediate model) from a course
 * courseId and courseName will be copied to their respective fields in the ICourse, but
 * course->studentIds will be converted from pointers to student ID numbers and stored as such
 */
ICourse ICourse_fromCourse(Course* course) {

    ICourse iCourse = {
            .courseId       = course->courseId,
            .studentsCount  = (byte) Course_studentsCount(course)
    };

    strncpy(iCourse.courseName, course->courseName, strlen(course->courseName));

    for(byte idx = 0; idx < iCourse.studentsCount; ++idx){
        iCourse.students[idx] = course->students[idx]->studentId;
    }

    return iCourse;
}

/*
 * Create a Course object from a primitive. Does not add students.
 */
Course ICourse_toCourse(ICourse* iCourse) {

    Course course = {
            .courseId = iCourse->courseId
    };

    strncpy(course.courseName, iCourse->courseName, strlen(iCourse->courseName));

    return course;
}

/*
 * Serialize course in to the receiver, starting at offset, and return the next NULL pointer index
 */
size ICourse_serialize(ICourse* course, byte* receiver, size offset) {

    size idx        = offset;
    byte nStudents  = (byte) course->studentsCount;
    byte nameSize   = (byte) strlen(course->courseName);

    // Segment 1 - number of students followed by as many student ID's
    receiver[idx++]  = nStudents;

    // -- Write so many student ID's
    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        receiver[idx++] = course->students[studentIdx];
    }

    // Segment 2 - Course ID
    receiver[idx++]  = course->courseId;

    // Segment 3 - actual name length followed as many character values
    receiver[idx++]  = nameSize;

    // -- Write name
    for(byte courseIdx = 0; courseIdx < nameSize; ++courseIdx) {
        // Assuming similar header definition, byte should be be an unsigned char.
        // ASCII values are always positive, so this should be OK.
        receiver[idx++] = (byte) course->courseName[courseIdx];
    }

    return idx;
}

size ICourse_deserialize(byte* data, size offset, ICourse* destination) {

    size idx        = offset;
    byte nStudents  = 0;
    byte nameSize   = 0;

    // Segment 1 - Read number of students, and read as many student ID's
    nStudents = data[idx++];
    destination->studentsCount = nStudents;

    // -- Read student ID's
    for(byte shiftIdx = 0; shiftIdx < nStudents; ++shiftIdx) {
        destination->students[shiftIdx] = data[idx++];
    }

    // Segment 2 - Read course ID
    destination->courseId = data[idx++];

    // Segment 3 - Read length of course name, followed by as many ASCII characters
    nameSize = data[idx++];

    // -- Read name
    for(byte charIdx = 0; charIdx < nameSize; ++charIdx) {
        destination->courseName[charIdx] = data[idx++];
    }

    // Append C ASCII null sentinel to string to prevent reading in to unused memory.
    destination->courseName[nameSize] = 0x00;

    // Sort Student ID's
    qsort(destination->students, nStudents, sizeof(byte), &compare_byte);

    return idx;
}

// ---- Student --------------------------------------------------------------------------------------------------------

/*
 * Serial format of Student (note that it is constructed from IStudent):
 *
 * B|[course ID's]|[course grades: B|[grades]]|B|B|[name]
 * - -------------                 - -------   - -  ----
 * | |                             | |         | |  |- sequence of bytes representing characters in name
 * | |                             | |         | |- length of name
 * | |                             | |         |- student ID
 * | |                             | |- sequence of bytes representing grades
 * | |                             |- number of grades in that specific course
 * | |- sequence of bytes representing course ID's
 * |- Number of courses
 *
 * Example:
 *
 * Student ID 0xA5 (#165)
 * Student Name: Test Student (54 65 73 74 20 53 74 75 64 65 6E 74)
 * Name length: 0x0C (12)
 *
 * 4 Courses:
 * - Course #0
 * - Course #1
 * - Course #2
 * - Course #3
 *
 * Course #0 has 0 grades
 * Course #1 has 3 grades
 * - 95 (0x5F)
 * - 80 (0x50)
 * - 60 (0x3C)
 * Course #2 has 2 grades
 * - 99  (0x5A)
 * - 100 (0x64)
 * Course #4 has 1 grade
 * - 100 (0x64)
 *
 *           |- 0x00 has 0x00 Grades
 *           --
 * 040001020300035F503C025A640164A50C646573742053747564656E74
 *  ---------  -------- ----- --- - -------------------------
 *  |          |        |     |   | | |- Sequence 0x0C bytes long containing ASCII data
 *  |          |        |     |   | |- 0x0C is name length
 *  |          |        |     |   |- 0xA5 is studentId
 *  |          |        |     |- 0x03 has 0x01 grades, 0x64
 *  |          |        |- 0x02 has 0x02 grades, 0x5A, 0x64
 *  |          |- 0x01 has 0x03 grades, 0x5F, 0x50, 0x3C
 *  |- 4 Courses, 0x00, 0x01, 0x02, 0x03
 *
 * Line split in to fields delimited by `;'
 * 04 00 01 02 03; 00; 03 5F 50 3C; 02 5A 64; 01 64; A5; 0C 54 65 73 74 20 53 74 75 64 65 6E 74
 */

typedef struct S_IStudent {

    /*
     * Courses by course ID
     */
    byte courses[4];

    byte coursesCount;

    /*
     * Multidimensional fixed array of unsigned 8-bit integers stores grades in relation to the course.
     * Where the first index specifies the course number (as it occurs in courses[_]).
     *
     * Effectively, this is 40 bytes long
     */
    byte grades[4][10];

    byte gradeCount[4];

    /**
    * 2-byte integer describes student ID.
    * This gives the range of ID's [0, 255], which far exceeds the specifications.
    *
    * This will be used to identify the student in the serialization process.
    */
    byte studentId;

    /**
    * Allow for up to 64 characters in a student name.
    * This should be more than enough.
    */
    char studentName[255];

} IStudent;

/*
 * GNU C Comparator for IStudent by studentId
 */
int IStudent_compareByID(const void* a, const void* b) {
    return ((IStudent*)a)->studentId - ((IStudent*)b)->studentId;
}

/*
 * Initializes an IStudent (primitive student serialization intermediate model) from a student
 * studentId and studentName will be copied, along with the grade array for each non-null pointer
 */
IStudent IStudent_fromStudent(Student* student) {

    IStudent iStudent = {
            .studentId      = student->studentId,
            .coursesCount   = (byte) Student_coursesCount(student),
    };

    strncpy(iStudent.studentName, student->studentName, strlen(student->studentName));

    for(byte idx = 0; idx < iStudent.coursesCount; ++idx) {
        iStudent.courses[idx]       = student->courses[idx].course->courseId;
        iStudent.gradeCount[idx]    = (byte) student->courses[idx].gradeCount;
        memcpy(iStudent.grades[idx], student->courses[idx].grades, sizeof(student->courses[idx].grades));
    }

    return iStudent;
}

/*
 * Initialize a Student from an intermediary student. Does not associate courses(!!).
 */
Student IStudent_toStudent(IStudent* iStudent) {

    size nCourses  = iStudent->coursesCount;

    Student student = {
            .studentId  = iStudent->studentId,
    };

    // Copy student name
    strncpy(student.studentName, iStudent->studentName, strlen(iStudent->studentName));

    // For each course, copy grades from the primitive model, to the non-primitive model
    for(byte idx = 0; idx < nCourses; ++idx) {
        student.courses[idx].gradeCount = iStudent->gradeCount[idx];
        memcpy(student.courses[idx].grades, iStudent->grades[idx], sizeof(iStudent->grades[idx]));
    }

    return student;
}

size IStudent_serialize(IStudent* student, byte* receiver, size offset) {

    size idx            = offset;
    byte nCourses       = student->coursesCount;
    byte nameSize       = (byte) strlen(student->studentName);

    // Segment 1 - nCourses followed by as many Course ID's
    receiver[idx++] = nCourses;

    // -- Write Course ID's
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        receiver[idx++] = student->courses[courseIdx];
    }

    // Segment 2 - for nCourses, write the number of grades followed by as many bytes representing each grade
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {

        // Get the length of the relative course signified by courseIdx (grades[courseIdx] corresponds to courses[courseIdx] grades)
        byte nGrades = student->gradeCount[courseIdx];
        receiver[idx++] = nGrades;

        // For each grade, write the grade as one byte
        for(byte gradeIdx = 0; gradeIdx < nGrades; ++gradeIdx){
            receiver[idx++] = student->grades[courseIdx][gradeIdx];
        }
    }

    // Segment 3 - Student ID
    receiver[idx++]   = student->studentId;

    // Segment 4 - Length of name, followed by ASCII name
    receiver[idx++]   = nameSize;

    // -- Write name
    for(byte charIdx = 0; charIdx < nameSize; ++charIdx) {
        receiver[idx++] = (byte) student->studentName[charIdx];
    }

    return idx;
}

size IStudent_deserialize(byte* data, size offset, IStudent* destination) {

    size idx        = offset;
    byte nCourses   = 0;
    byte nameSize   = 0;

    // Segment 1 - Read course count, followed by as many course ID's
    nCourses = data[idx++];
    destination->coursesCount = nCourses;

    // -- Read course ID's
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        destination->courses[courseIdx]  = data[idx++];
    }

    // Segment 2 - Read grades for each course
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {

        // Get the number of grades associated with this course
        byte nGrades    = data[idx++];

        // Copy each grade, if any, in to the associated index
        for(byte gradeIdx = 0; gradeIdx < nGrades; ++gradeIdx) {
            destination->grades[courseIdx][gradeIdx] = data[idx++];
        }
    }

    // Segment 3 - Read student ID
    destination->studentId = data[idx++];

    // Segment 4 - Get length of student's name and read as many characters
    nameSize = data[idx++];

    // -- Read characters in name
    for(byte charIdx = 0; charIdx < nameSize; ++charIdx) {
        destination->studentName[charIdx] = (char) data[idx++];
    }

    // Append C ASCII null sentinel to string to prevent reading in to unused memory.
    destination->studentName[nameSize] = 0x00;

    // Final Touch: Sort courseId's

    qsort(destination->courses, nCourses, sizeof(byte), &compare_byte);

    return idx;
}

// ---------------------------------------------------------------------------------------------------------------------

// ---- Exposed Interface ----------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------

SerializationStatus GradeBook_serialize(GradeBook* gradeBook, byte* buffer) {

    const size buffSize = sizeOfGradeBook(gradeBook);

    byte tmpBuffer[buffSize];

    const size nCourses  = gradeBook->coursesCount;
    const size nStudents = gradeBook->studentsCount;

    // Before beginning serialization, check for references to unknown Courses and Students
    // -----------------------------------------------------------------------------------------------------------------

    /*
     * Iterate through each course->student in the GradeBook, and insure that a student with a matching student ID is in the
     * GradeBook.
     */
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        size nCourseStudents = Course_studentsCount(&gradeBook->courses[courseIdx]);

        for(byte studentIdx = 0; studentIdx < nCourseStudents; ++studentIdx) {
            if(!Array_Contains(gradeBook->students, gradeBook->courses[courseIdx].students[studentIdx], nStudents, sizeof(Student), &Student_compareById)) {
                printf("%s contains an illegal reference to %s, which does not reside in the open GradeBook\n",
                        Course_toString(&gradeBook->courses[courseIdx]), Student_toString(gradeBook->courses[courseIdx].students[studentIdx]));
                return ILLEGAL_STUDENT_ID;
            }
        }
    }

    /*
     * Iterate through each student->course in the GradeBook, and insure that a course with a matching course ID is in the
     * GradeBook.
     */
    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        size nStudentCourses = Student_coursesCount(&gradeBook->students[studentIdx]);

        for(byte courseIdx = 0; courseIdx < nStudentCourses; ++courseIdx) {
            if(!Array_Contains(gradeBook->courses, gradeBook->students[studentIdx].courses[courseIdx].course, nCourses, sizeof(Course), &Course_compareById)) {
                printf("%s contains an illegal reference to %s, which does not reside in the open GradeBook\n",
                        Student_toString(&gradeBook->students[studentIdx]), Course_toString(gradeBook->students[studentIdx].courses[courseIdx].course));
                return ILLEGAL_COURSE_ID;
            }
        }
    }

    // Create serial grade book structure with course and student ID's
    // -----------------------------------------------------------------------------------------------------------------

    IGradeBook serialBook = {
            .coursesCount  = (byte) gradeBook->coursesCount,
            .studentsCount = (byte) gradeBook->studentsCount
    };

    for(byte idx = 0; idx < serialBook.coursesCount; ++idx) {
        serialBook.courses[idx] = gradeBook->courses[idx].courseId;
    }

    for(byte idx = 0; idx < serialBook.studentsCount; ++idx) {
        serialBook.students[idx] = gradeBook->students[idx].studentId;
    }

    // Create serial course structures
    // -----------------------------------------------------------------------------------------------------------------

    ICourse coursePrim[serialBook.coursesCount];

    for(byte idx = 0; idx < serialBook.coursesCount; ++idx) {
        coursePrim[idx] = ICourse_fromCourse(&gradeBook->courses[idx]);
    }

    // Create serial student structures
    // -----------------------------------------------------------------------------------------------------------------

    IStudent studentPrim[serialBook.studentsCount];

    for(byte idx = 0; idx < serialBook.studentsCount; ++idx) {
        studentPrim[idx] = IStudent_fromStudent(&gradeBook->students[idx]);
    }

    // Fill buffer
    // -----------------------------------------------------------------------------------------------------------------

    register size idx = 0;

    for(byte shift = 0; shift < NMEMBERS(GRADEBOOK_MAGIC, byte); ++shift){
        tmpBuffer[idx++] = GRADEBOOK_MAGIC[shift];
    }

    // Serialize the GradeBook index, update IDX to reflect the address of the next available byte
    idx = IGradeBook_serialize(&serialBook, tmpBuffer, idx);

    // For each course, serialize the course, and update IDX to reflect the address of the next available byte
    for(byte courseIdx = 0; courseIdx < serialBook.coursesCount; ++courseIdx) {
        if(idx + sizeOfCourse(&gradeBook->courses[courseIdx]) > buffSize) {
            printf("Course Serialization: IDX %lu of %lu: Not enough space left. Exiting.", idx, buffSize);
            return SHORT_BUFFER;
        }
        idx = ICourse_serialize(&coursePrim[courseIdx], tmpBuffer, idx);
    }

    // For each student, serialize the student, and update IDX to reflect the address of the next available byte
    for(byte studentIdx = 0; studentIdx < serialBook.studentsCount; ++studentIdx) {
        if(idx + sizeOfStudent(&gradeBook->students[studentIdx]) > buffSize) {
            printf("Student Serialization: IDX %lu of %lu: Not enough space left. Exiting. \n", idx, buffSize);
            return SHORT_BUFFER;
        }
        idx = IStudent_serialize(&studentPrim[studentIdx], tmpBuffer, idx);
    }

    // Copy temporary buffer to real buffer
    // -----------------------------------------------------------------------------------------------------------------

    memcpy(buffer, &tmpBuffer, buffSize);

    return SUCCESS;
}

SerializationStatus GradeBook_deserialize(byte* serialData, GradeBook* destination) {

    size idx            = 0;
    IGradeBook index    = {};

    // Read and validate (as we go) the magic
    for(byte magicIdx = 0; magicIdx < NMEMBERS(GRADEBOOK_MAGIC, byte); ++magicIdx) {
        if(GRADEBOOK_MAGIC[magicIdx] != serialData[idx++]){
            printf("Bad magic! buffer[%lu] (%02x) != %02x\n", idx, serialData[idx - 1], GRADEBOOK_MAGIC[magicIdx]);
            return BAD_MAGIC;
        }
    }

    // Read the GradeBook, and update IDX to the next unread byte
    idx = IGradeBook_deserialize(serialData, idx, &index);

    // Courses ---------------------------------------------------------------------------------------------------------

    // For each course ID in the GradeBook, read a course, and move IDX past the course
    destination->coursesCount   = index.coursesCount;
    const size nCourses         = destination->coursesCount;

    // -- Allocate some course pointers
    ICourse iCourses[nCourses];

    // -- Assign said pointers
    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        idx = ICourse_deserialize(serialData, idx, &iCourses[courseIdx]);
    }

    // -- Sort iCourses according to ICourse_compareByID comparator
    qsort(iCourses, nCourses, sizeof(ICourse), &ICourse_compareByID);

    // Students --------------------------------------------------------------------------------------------------------

    // For each student ID in the GradeBook, read a student, and move IDX past the student
    destination->studentsCount  = index.studentsCount;
    const size nStudents        = destination->studentsCount;

    // -- Allocate some student pointers
    IStudent iStudents[nStudents];

    // -- Assign said pointers
    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        idx = IStudent_deserialize(serialData, idx, &iStudents[studentIdx]);
    }

    // -- Sort iStudents according to IStudent_compareByID comparator
    qsort(iStudents, nStudents, sizeof(IStudent), &IStudent_compareByID);

    // Initialize Courses, Students. Sanitize ID's ---------------------------------------------------------------------

    // Check Course ID's. Construct Courses from ICourses.
    Course courses[nCourses];

    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        if(!Array_Contains(index.courses, &iCourses[courseIdx].courseId, index.coursesCount, sizeof(byte), &compare_byte)) {
            printf("ICourse has unreferenced id %u when deserializing\n", iCourses[courseIdx].courseId);
            return ILLEGAL_COURSE_ID;
        }

        courses[courseIdx]      = ICourse_toCourse(&iCourses[courseIdx]);
    }

    // Check Student ID's. Construct Students from IStudents.
    Student students[nStudents];

    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        if(!Array_Contains(index.students, &iStudents[studentIdx].studentId, index.studentsCount, sizeof(byte), &compare_byte)) {
            printf("IStudent has unreferenced id %u when deserializing\n", iStudents[studentIdx].studentId);
            return ILLEGAL_STUDENT_ID;
        }

        students[studentIdx]    = IStudent_toStudent(&iStudents[studentIdx]);
    }

    // Copy initialized students, courses, to destination GradeBook.

    memcpy(destination->courses, courses, nCourses * sizeof(Course));
    memcpy(destination->students, students, nStudents * sizeof(Student));

    // Fill cross-model references

    for(byte courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        // Because both course models have their students sorted by studentId, we can zip through and relate courses
        size nCourseStudents    = iCourses[courseIdx].studentsCount;

        for(byte studentIdx = 0; studentIdx < nCourseStudents; ++studentIdx) {

            /*
             * Create a temporary student structure to perform a binary search
             */
            Student key = { .studentId = iCourses[courseIdx].students[studentIdx] };

            /*
             * Search & add student
             */
            destination->courses[courseIdx].students[studentIdx] = bsearch(&key, destination->students, nStudents, sizeof(Student), &Student_compareById);

            /*
             * Check that the student reference is valid
             */
            if(!destination->courses[courseIdx].students[studentIdx]) {
                printf("Course %s references an illegal student ID: %u at index %u \n",
                        Course_toString(&destination->courses[courseIdx]), iCourses[courseIdx].students[studentIdx], studentIdx);

                return ILLEGAL_STUDENT_ID;
            }
        }
    }

    for(byte studentIdx = 0; studentIdx < nStudents; ++studentIdx) {
        // Once again, sorted arrays, etc..
        size nStudentCourses = iStudents[studentIdx].coursesCount;
        for(byte courseIdx = 0; courseIdx < nStudentCourses; ++courseIdx) {

            /*
             * Create a temporary course structure to perform a binary search
             */
            Course key = { .courseId = iStudents[studentIdx].courses[courseIdx] };

            /*
             * Search and add student
             */
            destination->students[studentIdx].courses[courseIdx].course =
                    bsearch(&key, destination->courses, nCourses, sizeof(Course), &Course_compareById);

            /*
             * Check that the course reference is valid
             */
            if(!destination->students[studentIdx].courses[courseIdx].course) {
                printf("Student %s references an illegal course ID: %u at index %u \n",
                        Student_toString(&destination->students[studentIdx]), iStudents[studentIdx].courses[courseIdx], courseIdx);

                return ILLEGAL_COURSE_ID;
            }
        }
    }


    return SUCCESS;
}

/*
 * For a description of the sizing algorithm for Student, see IStudent_serialize
 */
size sizeOfStudent(Student* student) {

    size nCourses      = Student_coursesCount(student);
    size sName         = strlen(student->studentName);
    size nGrades       = 0;

    for(size idx = 0; idx < nCourses; ++idx){
        nGrades += student->courses[idx].gradeCount;
    }

    return (1 + nCourses) + (1 + sName) + (nCourses + nGrades) + 1;
}

/*
 * For a description of the sizing algorithm for Course, see ICourse_serialize
 */
size sizeOfCourse(Course* course) {

    size nStudents  = Course_studentsCount(course);
    size sName      = strlen(course->courseName);

    return (1 + nStudents) + (1 + sName) + 1;
}

/*
 * This approximates the output size of a WHOLE GradeBook tree.
 * This is done by adding (one plus the number of courses), and (one plus the number of students),
 * along with the accumulated sum of the sizes of all students, and courses.
 *
 * The reason for adding one to the length (NOT SIZE) of the course and student arrays as that each course and each
 * student will be represent by one byte in a sequence of bytes N students or courses long at serialization.
 * Among other ways to determine how many students or courses to read is to prepend the number of these numbers to read
 * to the sequence. This is a good solution in this case, as nStudents < 100 and nCourses < 25, while the maximum value
 * that may be stored in one unsigned byte, 255, far exceeds these limitations.
 *
 * In addition, the size of the magic number, an identifier preceding all serialized data, is added to this.
 */
size sizeOfGradeBook(GradeBook* book) {
    size nCourses           = book->coursesCount;
    size nStudents          = book->studentsCount;
    size accumCourseSize    = 0;
    size accumStudentSize   = 0;

    for(byte idx = 0; idx < nCourses; ++idx) {
        accumCourseSize += sizeOfCourse(&book->courses[idx]);
    }

    for(byte idx = 0; idx < nStudents; ++idx) {
        accumStudentSize += sizeOfStudent(&book->students[idx]);
    }

    return (1 + nCourses) + (1 + nStudents) + accumCourseSize + accumStudentSize + NMEMBERS(GRADEBOOK_MAGIC, byte);
}

size sizeOfGradeBookOnly(GradeBook* book) {
    size nCourses  = book->coursesCount;
    size nStudents = book->studentsCount;

    return (1 + nCourses) + (1 + nStudents);
}
