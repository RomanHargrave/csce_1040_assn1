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
#include "models.h"

const grade MAX_GRADE = 0xFF;
const grade MIN_GRADE = 0x00;

// GradeBook -----------------------------------------------------------------------------------------------------------

const char* GradeBook_stringFormat = "GradeBook{ .courses[%02u], .students[%03u] }";

char* GradeBook_toString(GradeBook* book) {
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
    if(index < book->coursesCount){
        Course* replacement = &book->courses[--book->coursesCount];
        memmove(&book->courses[index], replacement, sizeof(Course));
        memset(&book->courses[book->coursesCount], 0, sizeof(Course));
        GradeBook_sortCourses(book);
    }
    return book->coursesCount;
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
        if(&book->students[idx] == student) {
            return GradeBook_removeStudentIndex(book, idx);
        }
    }
    return book->studentsCount;
}

size GradeBook_removeStudentIndex(GradeBook* book, size index) {
    if(index < book->studentsCount) {
        Student* replacement = &book->students[--book->studentsCount];
        memmove(&book->students[index], replacement, sizeof(Student));
        memset(&book->students[book->studentsCount], 0, sizeof(Student));
        GradeBook_sortStudents(book);
    }
    return book->studentsCount;
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
    char* nameFormatted[strlen(Student_stringFormat) + strlen(course->courseName)];
    sprintf((char*) nameFormatted, Course_stringFormat, course->courseId, course->courseName, Course_studentsCount(course));

    return nameFormatted;
}

size Course_studentsCount(Course* course) {
    return PtrArray_CountSane((void**) course->students, NMEMBERS(course->students, Student*));
}

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
    char* nameFormatted[strlen(Student_stringFormat) + strlen(student->studentName) + 15];
    sprintf((char*) nameFormatted, Student_stringFormat, student->studentId, student->studentName, Student_coursesCount(student));

    return nameFormatted;
}

size Student_coursesCount(Student* student) {
    return PtrArray_CountSane((void**) student->courses, NMEMBERS(student->courses, Course*));
}

long Student_courseIndex(Student* student, Course* course) {
    size nCourses = Student_coursesCount(student);
    for(size idx = 0; idx < nCourses; ++idx) {
        if(Course_compareById(course, student->courses[idx]) == 0) {
            return idx;
        }
    }

    return -1;
}

bool Student_removeCourse(Student* student, Course* course) {

    long courseIdx = Student_courseIndex(student, course);
    if(courseIdx < 0) return false;

    size nCourses = Student_coursesCount(student);

    if(courseIdx < NMEMBERS(student->courses, Course*) - 1) {
        size moveAmt = nCourses - courseIdx - 1;
        memmove(student->courses + courseIdx, student->courses + courseIdx + 1, sizeof(Course*) * moveAmt);
        memmove(student->grades + courseIdx, student->courses + courseIdx + 1, sizeof(grade*) * moveAmt);
        memmove(student->gradeCount + courseIdx, student->gradeCount + courseIdx + 1, sizeof(byte) * moveAmt);
    } else {
        student->courses[courseIdx] = NULL;
    }

    return true;
}

bool Student_addCourse(Student* student, Course* course) {

    size nCourses = Student_coursesCount(student);
    if(nCourses >= NMEMBERS(student->courses, Course*)) return false;

    student->courses[nCourses] = course;
    memset(student->grades[nCourses], 0, NMEMBERS(student->grades[nCourses], grade) * sizeof(grade));
    student->gradeCount[nCourses] = 0;

    return true;
}