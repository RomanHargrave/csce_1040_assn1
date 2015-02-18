/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Default implementation of prototypes in `grading.h`
 */

#include <string.h>
#include <strings.h>
#include <slcurses.h>
#include "grading.h"


// ---- Grade Array Manipulation ---------------------------------------------------------------------------------------

int GradeArray_sum(grade gradePtr[], size nGrades) {
    int accumulator = 0;

    for(size idx = 0; idx < nGrades; ++idx){
        accumulator += *gradePtr++;
    }

    return accumulator;
}

float GradeArray_average(grade gradePtr[], size nGrades) {
    return nGrades > 0 ? (float) GradeArray_sum(gradePtr, nGrades) / (float) nGrades : 0;
}


float Student_averageGrade(Student* student) {
    size nCourses = Student_coursesCount(student);
    float gradeAccum = 0;
    for(size courseIdx = 0; courseIdx < nCourses; ++courseIdx) {
        gradeAccum += GradeArray_average(student->courses[courseIdx].grades, student->courses[courseIdx].gradeCount);
    }
    return nCourses > 0 ? gradeAccum / nCourses : 0;
}

float Course_averageGrade(Course* course) {
    size nStudents      = Course_studentsCount(course);
    float gradeAccum    = 0;
    for(size idx = 0; idx < nStudents; ++idx) {
        Student* student = course->students[idx];
        long indexInStudent = Student_courseIndex(student, course);
        if(indexInStudent < 0) {
            // Should we log an error?
            continue;
        }

        gradeAccum += GradeArray_average(student->courses[indexInStudent].grades, student->courses[indexInStudent].gradeCount);
    }

    return nStudents > 0 ? gradeAccum / nStudents : 0;
}

grade GradeArray_smallest(grade gradePtr[], size nGrades) {
    grade cache = MAX_GRADE;

    for(size idx = 0; idx < nGrades; ++idx) {
        cache = (cache > *gradePtr++) ? *gradePtr : cache;
    }

    return cache;
}

grade GradeArray_largest(grade gradePtr[], size nGrades) {
    grade cache = 0;

    for(size idx = 0; idx < nGrades; ++idx) {
        cache = (cache < *gradePtr++) ? *gradePtr : cache;
    }

    return cache;
}

bool GradeArray_remove(grade gradePtr[], size nGrades, size index) {

    if(index > (nGrades - 1)) return  false;

    if(index == (nGrades - 1)) {
        gradePtr[index] = 0;
    } else {
        gradePtr[index] = gradePtr[nGrades - 1];
        gradePtr[nGrades - 1] = 0;
    }

    return true;

}

grade GradeArray_addNew(grade gradePtr[], size nGrades, grade newGrade) {

    /*
     * Get the grade from the pointer of greatest value
     */
    grade lastGrade = gradePtr[nGrades - 1];

    #ifdef __USE_TRICKS

    for(size idx = 0; idx < nGrades; ++idx) {
        /*
         * Fun order-of-operations (ab)use:
         * Assign the value at gradePtr+1 to the value of gradePtr
         * This happens because the expression `*gradePtr` (dereference gradePtr) is decomposed by the compiler before
         * the value of gradePtr is incremented.
         */
        *++gradePtr = *gradePtr;
    }

    #else



    #endif

    /*
     * Put the new grade at the pointer of least value
     */
    gradePtr[0] = newGrade;

    return lastGrade;
}


// ---- Enrollment Manipulation ----------------------------------------------------------------------------------------

grade Enrollment_addGrade(StudentEnrollment* enrollment, grade newGrade) {

    if(enrollment->gradeCount >= NMEMBERS(enrollment->grades, grade)) {
        grade removed = enrollment->grades[enrollment->gradeCount -1];
        memmove(enrollment->grades + 1, enrollment->grades, (enrollment->gradeCount - 1));
        return removed;
    } else {
        enrollment->grades[enrollment->gradeCount++] = newGrade;
        return 0;
    }
}

bool Enrollment_removeGrade(StudentEnrollment* enrollment, size index) {

    if(index >= enrollment->gradeCount || enrollment->gradeCount == 0) return false;


    if(index == (enrollment->gradeCount - 1)) {
        enrollment->grades[index] = 0;
    } else {
        // Shift left from idx + 1 by 1, overwriting idx
        memmove(enrollment->grades + index, enrollment->grades + index + 1, enrollment->gradeCount - index - 1);

        --enrollment->gradeCount;

        // Zero out the tail end of the array from the first unused element to the end of the array.
        memset(enrollment->grades + enrollment->gradeCount, 0x00,
                sizeof(grade) * (NMEMBERS(enrollment->grades, grade) - enrollment->gradeCount));
    }

    --enrollment->gradeCount;

    return true;
}

float Enrollment_average(StudentEnrollment* enrollment) {
    return GradeArray_average(enrollment->grades, enrollment->gradeCount);
}