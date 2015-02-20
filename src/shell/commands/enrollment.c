/*
 * Roman Hargrave, 2015.
 * CSCE 1040, ***REMOVED***
 *
 * Implements the commands used when managing student/course relationships
 */

#include <string.h>
#include <stdint.h>
#include "../../models/models.h"
#include "command.h"
#include "../../debug.h"
#include "../../grading.h"

static const char* ACTION_ADD = "add";
static const char* ACTION_DEL = "rm";

Course* Command_enroll_grade_findCourse(GradeBook* book, byte cid) {
    return bsearch(&(Course){.courseId = cid}, book->courses, book->coursesCount, sizeof(Course), &Course_compareById);
}

Student* Command_enroll_grade_findStudent(GradeBook* book, byte sid) {
    return bsearch(&(Student){.studentId = sid}, book->students, book->studentsCount, sizeof(Student), &Student_compareById);
}

ShellReturn Command_enroll(char* args, GradeBook* gradeBook) {

    char* action    = strtok(args, " ");
    char* sidString = strtok(NULL, " ");
    char* cidString = strtok(NULL, " ");

    if(!action ||  !sidString || !cidString) {
        printf("You must input an action, course ID, and student ID to manage enrollment\n");
        return SR_FAILURE;
    }

    int cid = atoi(cidString);
    int sid = atoi(sidString);

    if(Course_isValidId(cid) == false || Student_isValidId(sid) == false) {
        printf( "Valid course and student ID's must be specified\n");
        return SR_FAILURE;
    }

    d_printf("Looking up course %i, student %i\n", cid, sid);

    Course* course      = Command_enroll_grade_findCourse(gradeBook, (byte) cid);
    Student* student    = Command_enroll_grade_findStudent(gradeBook, (byte) sid);

    char* studentString = Student_toString(student);
    char* courseString  = Course_toString(course);

    d_printf("Found %s, %s\n", courseString, studentString);

    free(courseString);
    free(studentString);

    if(!course || !student) {
        printf("Student or course not found\n");
        return SR_FAILURE;
    }

    if(strcasecmp(action, ACTION_ADD) == 0) {

        bool success = Course_addStudent(course, student);

        if(success == true) {
            printf("Student added to course\n");
            return SR_SUCCESS;
        } else {
            printf("Student could not be added. Perhaps the course is full?\n");
            return SR_FAILURE;
        }
    } else if(strcasecmp(action, ACTION_DEL) == 0) {

        bool success = Course_remStudent(course, student);

        if(success == true) {
            printf("Student removed from course\n");
            return SR_SUCCESS;
        } else {
            printf("Student could not be removed. Is the student enrolled?\n");
            return SR_FAILURE;
        }
    } else {
        printf("Uknown operation `%s`\n", action);
    }

    return SR_FAILURE;
}

ShellReturn Command_grade(char* args, GradeBook* gradeBook) {

    char* action    = strtok(args, " ");
    char* sidString = strtok(NULL, " ");
    char* cidString = strtok(NULL, " ");
    char* numberStr = strtok(NULL, " ");

    if(!action | !sidString | !cidString | !numberStr) {
        d_printf("Args passed (in order) %s , %s , %s , %s\n", action, sidString, cidString, numberStr);
        printf("You must specifiy an action (add, rm) a student ID, a course ID, and a grade or index\n");
        return SR_FAILURE;
    }

    int sid             = atoi(sidString);
    int cid             = atoi(cidString);
    int gradeOrIndex    = atoi(numberStr);

    if(!Student_isValidId(sid) || !Course_isValidId(cid)) {
        printf("You must enter both a valid student ID, and a valid course ID\n");
        return SR_FAILURE;
    }

    if(((gradeOrIndex < MIN_GRADE) || (gradeOrIndex > MAX_GRADE)) && (strcmp(action, ACTION_ADD) == 0)) {
        printf("You must enter a grade between %u and %u\n", MIN_GRADE, MAX_GRADE);
        return SR_FAILURE;
    } else if(gradeOrIndex < 0) {
        printf("You must enter an index greater than or equal to 0\n");
        return SR_FAILURE;
    }

    Student* student    = Command_enroll_grade_findStudent(gradeBook, (byte) sid);
    Course* course      = Command_enroll_grade_findCourse(gradeBook, (byte) cid);

    if(!student) {
        printf("No student with the id %i exists\n", sid);
        return SR_FAILURE;
    }

    if(!course) {
        printf("No course with the id %i exists\n", cid);
        return SR_FAILURE;
    }

    long indexInStudent = Student_courseIndex(student, course);

    if(indexInStudent < 0) {
        printf("The student is not not enrolled in the specified course\n");
        return SR_FAILURE;
    }

    StudentEnrollment* enrollment = &student->courses[indexInStudent];

    if(!enrollment) {
        printf("Invalid enrollment! Contact support with a coredump immediately.\n");
        printf("idx=%l ptr=0x%x\n (%lu)\n", indexInStudent, (intptr_t) enrollment, (intptr_t) enrollment);
        return SR_FAILURE;
    }

    if(strcmp(action, ACTION_ADD) == 0) {

        Enrollment_addGrade(enrollment, (grade) gradeOrIndex);

        printf( "Student grades in course updated\n"
                "Average in course is now %f.\n", Enrollment_average(enrollment));

    } else if(strcmp(action, ACTION_DEL) == 0) {

        if(gradeOrIndex >= enrollment->gradeCount) {
            printf("Speicifed grade index falls outside the grade count for this student\n");
            return SR_FAILURE;
        }

        if(Enrollment_removeGrade(enrollment, (size) gradeOrIndex) == true) {

            printf("The specified grade was removed from the student\n");

        } else {

            printf("Unable to remove grade (reason unknown)\n");

            return SR_FAILURE;
        }

    }

    return SR_SUCCESS;
}