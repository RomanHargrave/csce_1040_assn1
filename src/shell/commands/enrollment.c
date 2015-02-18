#include <string.h>
#include <strings.h>
#include "../../models/models.h"
#include "command.h"
#include "../../debug.h"

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

    return SR_FAILURE;
}