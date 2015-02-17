/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Models Header:
 *
 * Describes data models used in the GradeBook program.
 * Model specifications will be placed in comments adjacent to the model definition.
 *
 * This header is referenced in the following files:
 *
 * - grade.c    -> Program business logic
 * - model_io.c -> Model serialization logic
 * - models.c   -> Definitions of linked objects
 */

// Include Guard
#ifndef _H_MODELS
    #define _H_MODELS
    #include "../util.h"

// Begin Header "models" -----------------------------------------------------------------------------------------------


/*
 * Define the type used for numeric grades
 */
typedef byte grade;
extern const grade MAX_GRADE;
extern const grade MIN_GRADE;

/*
 * In order to deal with the issue of circular dependency between student and course models,
 * declare an opaque student and course, and redefine them later.
 */
typedef struct S_Student    Student;
typedef struct S_Course     Course;


// Course --------------------------------------------------------------------------------------------------------------

/*
 * Course model.
 *
 * Declares members:
 * - students: Points to students
 * - courseId: Course number, 0-255
 * - courseName: Course name, 16 characters max
 */
typedef struct S_Course {

    /**
    * Array of student pointers.
    * Serialization procedure behaves in the same manner as does that of courses[4] in student
    */
    Student* students[20];

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

} Course;

// -- Course Functions -------------------------------------------------------------------------------------------------

/*
 * Helper function that returns the number of students in a course
 */
size Course_studentsCount(Course* course);

/*
 * Function that, when called, returns the difference between the courseId's of two courses
 */
int Course_compareById(const void *a, const void *b);

bool Course_addStudent(Course* course, Student* student);

bool Course_remStudentIndex(Course* course, size index);

bool Course_remStudent(Course* course, Student* student);

/*
 * String format of a course. Displays only course id, and course name.
 * Defined in models.c
 */
extern const char* Course_stringFormat;

/*
 * Function that, when called, returns a string value representative of the course
 */
char* Course_toString(Course* course);

bool Course_isValidId(long number);

//#define Course_toString(course, string) sprintf(string, Course_stringFormat, course->courseId, course->courseName)

// Student -------------------------------------------------------------------------------------------------------------

typedef struct S_StudentEnrollment {

    Course* course;

    grade grades[10];

    size gradeCount;

} StudentEnrollment;

/*
 * Student model.
 *
 * Declares members
 * - courses: Course*[4]
 * - grades: unsigned char[4][10]
 * - studentId: one-byte student ID
 * - studentName: student name, max 64 characters
 */
typedef struct S_Student {

    /*
     * For the sake of serialization, we need to assume a fixed size for each model.
     * That being said, pointer size varies based on the processor architecture.
     * Luckily, when serializing, we will not need to worry about storing pointers,
     * as we can assume that for all intents and purposes, an 8-bit unsigned integer will be
     * assigned to each course.
     * I'll detail model serialization further in the serializer implementation.
     * This makes the effective size of this member at serialization-time 32 bits, or 4 bytes.
     */
    StudentEnrollment courses[4];

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

} Student;

// -- Student Functions ------------------------------------------------------------------------------------------------

/*
 * Helper function for student that returns the amount of defined course references the student has
 */
size Student_coursesCount(Student*);

/*
 * Function that, when called, returns a number representing difference between the studentId values of two students
 */
int Student_compareById(const void* a, const void* b);

/*
 * String format of a student. Displays only student id, and student name.
 * Defined in models.c
 */
extern const char* Student_stringFormat;

/*
 * Function that, when called, returns a string value representing the student
 */
char* Student_toString(Student* student);

long Student_courseIndex(Student* student, Course* course);

/*
 * These two functions should not be exposed to external callers, as they should be called by the Course_addStudent, and Course_removeStudent actions
 *
 * bool Student_removeCourse(Student* student, Course* course);
 *
 * bool Student_addCourse(Student* student, Course* course);
 */

bool Student_isValidId(long number);

//#define Student_toString(student, string) sprintf(string, Student_stringFormat, student->studentId, student->studentName)

// GradeBook -----------------------------------------------------------------------------------------------------------

/**
* The GradeBook model is the "trunk" of the GradeBook data architecture, in that it stores the actual data inside the
*   Course and Student models, while they simply reference each other.
*
* Due to the nature of C "arrays" and static allocation, the index also stores data about the number of
*   Courses and Students inside of it.
* As such, it is recommended to use the GradeBook_add(_) and GradeBook_remove(_) functions to perform operations
*   on actual data inside the GradeBook.
*/
typedef struct S_GradeBook {

    /*
     * Array of courses
     *
     * Size is 25 bytes at serialization
     */
    Course courses[25];

    /*
     * Tracks the number of courses present in the GradeBook.
     */
    size coursesCount;

    /*
     * Array of students
     *
     * Size is 100 bytes at serialization
     */
    Student students[100];

    /*
     * Tracks the number of students present in the GradeBook.
     */
    size studentsCount;

} GradeBook;

extern const char* GradeBook_stringFormat;

/*
 * Add a course to the GradeBook, and update necessary metadata. Return the next available index.
 */
size GradeBook_addCourse(GradeBook* book, Course course);

/*
 * Search for and remove a course, returns the next available index.
 */
size GradeBook_removeCourse(GradeBook* book, Course* course);

/*
 * Search for and remove a course at `index`.
 */
size GradeBook_removeCourseIndex(GradeBook* book, size index);

/*
 * Add a student to the GradeBook and update necessary metadata. Return the next available index.
 */
size GradeBook_addStudent(GradeBook* book, Student student);

/*
 * Remove a student by reference, and update data.
 */
size GradeBook_removeStudent(GradeBook* book, Student* student);

/*
 * Remove a course at `index`
 */
size GradeBook_removeStudentIndex(GradeBook* book, size index);

char* GradeBook_toString(GradeBook* book);

// End Header "models" -------------------------------------------------------------------------------------------------

#endif