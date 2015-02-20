/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Defines useful constants and functions for displaying tabular data about models.
 */

#ifndef _H_MODEL_DISPLAY
    #define _H_MODEL_DISPLAY
    #include "../models/models.h"

// Begin header "model display" ----------------------------------------------------------------------------------------

extern const char* GradeBook_COURSE_TABLE_COLUMNS[];

extern const size GradeBook_COURSE_COLUMN_COUNT;

extern const char* GradeBook_STUDENT_TABLE_COLUMNS[];

extern const size GradeBook_STUDENT_COLUMN_COUNT;

void GradeBook_courseTable(GradeBook* gradeBook, char* destination[][GradeBook_COURSE_COLUMN_COUNT]);

void GradeBook_studentsTable(GradeBook* gradeBook, char* destination[][GradeBook_STUDENT_COLUMN_COUNT]);

// Course --------------------------------------------------------------------------------------------------------------

extern const char* Course_STUDENT_COLUMNS[];

extern const size Course_STUDENT_COLUMNS_COUNT;

void Course_studentsTable(Course* course, char* table[][Course_STUDENT_COLUMNS_COUNT]);

// Student -------------------------------------------------------------------------------------------------------------

extern const char* Student_COURSE_COLUMNS[];

extern const size Student_COURSE_COLUMNS_COUNT;

void Student_coursesTable(Student* student, char* table[][Course_STUDENT_COLUMNS_COUNT]);

// End header "model display" ------------------------------------------------------------------------------------------

#endif