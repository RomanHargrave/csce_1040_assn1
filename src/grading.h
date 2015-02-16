/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Provides mathematics and array applications specific to the grading process
 */


#ifndef _H_GRADING
    #define _H_GRADING
    #include "models/models.h"


// Begin Header "grading.h" --------------------------------------------------------------------------------------------


// ---- List Applications ----------------------------------------------------------------------------------------------

/*
 * Apply a grading function to a grade array.
 */
int GradeArray_sum(grade[], size);

/*
 * Calculate the average grade in a grade array
 */
float GradeArray_average(grade[], size);

/*
 * Helper method that flattens the grade matrix of a student and averages that student's grades
 */
float Student_averageGrade(Student* student);

/*
 * Helper method that collects the course grades for every enrolled student and performs an average
 * across the flattened arrays.
 */
float Course_averageGrade(Course* course);

/*
 * Find the smallest grade in a grade array
 */
grade GradeArray_smallest(grade[], size);

/*
 * Find the largest grade in a grade array
 */
grade GradeArray_largest(grade[], size);

/*
 * Shifts all contained grades +1 position
 * Value pointed to by the greatest pointer in the array will be returned, and will not be in the array any longer
 */
grade GradeArray_addNew(grade[], size, grade);

// End Header "grading.h" ----------------------------------------------------------------------------------------------

#endif