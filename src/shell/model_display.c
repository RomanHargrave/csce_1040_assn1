#include <string.h>
#include "model_display.h"
#include "../grading.h"
#include "../tui.h"

const char* GradeBook_COURSE_TABLE_COLUMNS[] =
        {"Course ID", "Course Name", "Students", "Average Grade"};

const size GradeBook_COURSE_COLUMN_COUNT = 4;

const char* GradeBook_STUDENT_TABLE_COLUMNS[] =
        {"Student ID", "Student Name", "Courses", "Average Grade"};

const size GradeBook_STUDENT_COLUMN_COUNT = 4;

const char* Course_STUDENT_COLUMNS[] =
        {"Student ID", "Student Name", "Course Average", "Course Grades"};

const size Course_STUDENT_COLUMNS_COUNT = 4;

const char* Student_COURSE_COLUMNS[] =
        {"Course ID", "Course Name", "Average", "Grades"};

const size Student_COURSE_COLUMNS_COUNT = 4;

void GradeBook_courseTable(GradeBook* gradeBook, char* table[][GradeBook_COURSE_COLUMN_COUNT]) {

    for(size courseIdx = 0; courseIdx < gradeBook->coursesCount; ++courseIdx) {
        sprintf(table[courseIdx][0], "%03u", gradeBook->courses[courseIdx].courseId);
        strcpy(table[courseIdx][1], gradeBook->courses[courseIdx].courseName);
        sprintf(table[courseIdx][2], "%lu", Course_studentsCount(&gradeBook->courses[courseIdx]));
        sprintf(table[courseIdx][3], "%3.02f", Course_averageGrade(&gradeBook->courses[courseIdx]));
    }

}

void GradeBook_studentsTable(GradeBook* gradeBook, char* table[][GradeBook_STUDENT_COLUMN_COUNT]) {

    for(size studentIdx = 0; studentIdx < gradeBook->studentsCount; ++studentIdx) {
        sprintf(table[studentIdx][0], "%03u", gradeBook->students[studentIdx].studentId);
        strcpy(table[studentIdx][1], gradeBook->students[studentIdx].studentName);
        sprintf(table[studentIdx][2], "%lu", Student_coursesCount(&gradeBook->students[studentIdx]));
        sprintf(table[studentIdx][3], "%3.02f", Student_averageGrade(&gradeBook->students[studentIdx]));
    }

}

void gradeStringifier(const void* gVal, FILE* stream) {
    fprintf(stream, "%u", *(grade*)gVal);
}

void Course_studentsTable(Course* course, char* table[][Course_STUDENT_COLUMNS_COUNT]) {

    size nStudents = Course_studentsCount(course);

    for(size idx = 0; idx < nStudents; ++idx) {
        Student* student = course->students[idx];
        if(!student){
            strcpy(table[idx][0], "(null)");
            continue;
        }
        long selfIdx = Student_courseIndex(student, course);
        if(selfIdx < 0) continue;

        StudentEnrollment* enrollment = &student->courses[selfIdx];

        sprintf(table[idx][0], "%03u", student->studentId);
        strcpy(table[idx][1], student->studentName);
        sprintf(table[idx][2], "%3.02f", Enrollment_average(enrollment));
        char* gradeStr = Array_toString(enrollment->grades, enrollment->gradeCount, sizeof(grade), ", ", &gradeStringifier);
        strcpy(table[idx][3], gradeStr ? gradeStr : "None");
        free(gradeStr);
    }

}

void Student_coursesTable(Student* student, char* table[][Student_COURSE_COLUMNS_COUNT]) {

    size nCourses = Student_coursesCount(student);

    for(size idx = 0; idx < nCourses; ++idx) {
        Course* course = student->courses[idx].course;
        if(!course) {
            strcpy(table[idx][0], "(null)");
            continue;
        }

        StudentEnrollment* enrollment = &student->courses[idx];

        sprintf(table[idx][0], "%02u", course->courseId);
        strcpy(table[idx][1], course->courseName);
        sprintf(table[idx][2], "%3.02f", Enrollment_average(enrollment));
        char* gradeStr = Array_toString(enrollment->grades, enrollment->gradeCount, sizeof(grade), ", ", &gradeStringifier);
        strcpy(table[idx][3], (gradeStr ?  gradeStr : "None"));
        free(gradeStr);
    }

}
