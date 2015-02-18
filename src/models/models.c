/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Models Definitions:
 *
 * Implements linked functions and constants defined in models.h
 */

#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <slcurses.h>
#include "models.h"
#include "../grading.h"

const grade MAX_GRADE = 0xFF;
const grade MIN_GRADE = 0x00;

// Student -------------------------------------------------------------------------------------------------------------

int Student_compareById(const void *a, const void *b){
    if(!a | !b) {
      return (!a && !b) ? 0 : -1;
    } else {
        return ((Student*)a)->studentId - ((Student*)b)->studentId;
    }
}

const char* Student_stringFormat = "Student{ .studentId = %03u, .studentName = \"%s\", .courses[%u] }";

char* Student_toString(Student* student) {
    if(!student) return "(Student)null";
    char* nameFormatted[strlen(Student_stringFormat) + strlen(student->studentName) + 15];
    sprintf((char*) nameFormatted, Student_stringFormat, student->studentId, student->studentName, Student_coursesCount(student));

    return nameFormatted;
}

size Student_coursesCount(Student* student) {
    // How to write hard-to-follow C code in one easy line:
    for(size idx = 0; idx < NMEMBERS(student->courses, StudentEnrollment); ++idx) {
        if(!student->courses[idx].course) return idx;
    };

    return 0;
}

long Student_courseIndex(Student* student, Course* course) {
    size nCourses = Student_coursesCount(student);
    for(size idx = 0; idx < nCourses; ++idx) {
        if(Course_compareById(course, student->courses[idx].course) == 0) {
            return idx;
        }
    }

    return -1;
}

int StudentEnrollment_compareByCourseId(const void* a, const void* b) {
    if(!a || !b) {
        return (!a && !b) ? 0 : -1;
    } else if(!((StudentEnrollment*)a)->course || !((StudentEnrollment*)b)->course) {
        return (!((StudentEnrollment*)a)->course && !((StudentEnrollment*)b)->course) ? 0 : -1;
    } else {
        return ((StudentEnrollment*)a)->course->courseId - ((StudentEnrollment*)b)->course->courseId;
    }
}

bool Student_removeCourse(Student* student, Course* course) {

    long courseIdx = Student_courseIndex(student, course);
    if(courseIdx < 0) return false;

    size nCourses = Student_coursesCount(student);

    student->courses[courseIdx].course = NULL;
    student->courses[courseIdx].gradeCount = 0;
    memset(student->courses[courseIdx].grades, 0, sizeof(student->courses[courseIdx].grades));

    qsort(student->courses, NMEMBERS(student->courses, StudentEnrollment), sizeof(StudentEnrollment), &StudentEnrollment_compareByCourseId);

    return true;
}

bool Student_addCourse(Student* student, Course* course) {

    size nCourses = Student_coursesCount(student);
    if(nCourses >= NMEMBERS(student->courses, Course*)) return false;

    StudentEnrollment enrollment = {
            .course = course
    };

    memset(enrollment.grades, 0, NMEMBERS(enrollment.grades, grade) * sizeof(grade));
    enrollment.gradeCount = 0;

    memcpy(&student->courses[nCourses], &enrollment, sizeof(StudentEnrollment));

    return true;
}

bool Student_addGrade(Student* student, Course* course, grade grade) {

    long courseIndex = Student_courseIndex(student, course);
    if(courseIndex < 0) return false;

    StudentEnrollment* courseRel = &student->courses[courseIndex];

    GradeArray_addNew(courseRel->grades, NMEMBERS(courseRel->grades, grade), grade);
    courseRel->gradeCount++;

    return true;
}

// Course --------------------------------------------------------------------------------------------------------------

int Course_compareById(const void* a, const void* b){
    if(!a | !b){
        return (!a && !b) ? 0 : -1;
    } else {
        return ((Course*)a)->courseId - ((Course*)b)->courseId;
    }
}

const char* Course_stringFormat = "Course{ .courseId = %03u, .courseName = \"%s\", .students[%02u] }";

char* Course_toString(Course* course) {
    if(!course) return "(Course)null";
    char* nameFormatted[strlen(Course_stringFormat) + strlen(course->courseName)];
    sprintf((char*) nameFormatted, Course_stringFormat, course->courseId, course->courseName, Course_studentsCount(course));

    return nameFormatted;
}

size Course_studentsCount(Course* course) {
    return PtrArray_CountSane((void**) course->students, NMEMBERS(course->students, Student*));
}

bool Course_addStudent(Course* course, Student* student) {

    if(Course_studentsCount(course) >= NMEMBERS(course->students, Student*)) return false;

    course->students[Course_studentsCount(course)] = student;

    qsort(course->students, Course_studentsCount(course), sizeof(Student*), &Student_compareById);

    Student_addCourse(student, course);

    return true;
}

bool Course_remStudentIndex(Course* course, size index) {

    size initialSize = Course_studentsCount(course);

    course->students[index] = NULL;

    qsort(course->students, initialSize, sizeof(Student*), &Student_compareById);

    return true;
}

bool Course_remStudent(Course* course, Student* student) {
    size initialSize = Course_studentsCount(course);

    for(size idx = 0; idx < initialSize; ++idx) {
        if(Student_compareById(student, course->students[idx]) == 0){
            return Course_remStudentIndex(course, idx);
        }
    }

    Student_removeCourse(student, course);

    return false;
}

// GradeBook -----------------------------------------------------------------------------------------------------------

const char* GradeBook_stringFormat = "GradeBook{ .courses[%02u], .students[%03u] }";

char* GradeBook_toString(GradeBook* book) {
    if(!book) return "(GradeBook)null";
    char* nameFormatted[strlen(Student_stringFormat)];
    sprintf(nameFormatted, GradeBook_stringFormat, book->coursesCount, book->studentsCount);

    return nameFormatted;
}

// -- Course Management ------------------------------------------------------------------------------------------------

void GradeBook_sortCourses(GradeBook* book) {
    qsort(book->courses, book->coursesCount, sizeof(Course), &Course_compareById);
}

size GradeBook_addCourse(GradeBook* book, Course course) {
    book->courses[book->coursesCount++] = course;
    GradeBook_sortCourses(book);
    return book->coursesCount;
}

size GradeBook_removeCourse(GradeBook* book, Course* course) {
    // Lovely O(N)+ search oh my
    for(size idx = 0; idx < book->coursesCount; ++idx) {
        if(&book->courses[idx] == course) {
            return GradeBook_removeCourseIndex(book, idx);
        }
    }
    return book->coursesCount;
}

size GradeBook_removeCourseIndex(GradeBook* book, size index) {
    if(index == book->coursesCount - 1){
        --book->coursesCount;
    } else if(index < book->coursesCount) {
        Course* replacement = &book->courses[book->coursesCount - 2];

        --book->coursesCount;

        memmove(&book->courses[index], replacement, sizeof(Course));

        GradeBook_sortCourses(book);
    }
    return book->coursesCount;
}


bool Course_isValidId(long number) {
    return (number >= BYTE_MIN) && (number <= BYTE_MAX);
}

// -- Student Management -----------------------------------------------------------------------------------------------

void GradeBook_sortStudents(GradeBook* book) {
    qsort(book->students, book->studentsCount, sizeof(Student), &Student_compareById);
}

size GradeBook_addStudent(GradeBook* book, Student student) {
    book->students[book->studentsCount++] = student;
    GradeBook_sortStudents(book);
    return book->studentsCount;
}

size GradeBook_removeStudent(GradeBook* book, Student* student) {
    for(size idx = 0; idx < book->studentsCount; ++idx) {
        if(Student_compareById(student, &book->students[idx]) == 0) {
            return GradeBook_removeStudentIndex(book, idx);
        }
    }
    return book->studentsCount;
}

size GradeBook_removeStudentIndex(GradeBook* book, size index) {
    if(index < book->studentsCount) {
        Student* original = &book->students[index];
        size nCourses = Student_coursesCount(original);
        for(size idx = 0; idx < nCourses; ++idx) {
            Course_remStudent(original->courses[idx].course, original);
        }
        Student* replacement = &book->students[--book->studentsCount];
        memmove(&book->students[index], replacement, sizeof(Student));
        memset(&book->students[book->studentsCount], 0, sizeof(Student));
        GradeBook_sortStudents(book);
    }
    return book->studentsCount;
}

bool Student_isValidId(long number) {
    return (number >= BYTE_MIN) && (number <= BYTE_MAX);
}
