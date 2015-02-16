/*
 * Roman Hargrave, ***REMOVED***
 * No License Declared
 *
 * Model IO Header:
 *
 * Defines model serialization function signatures and helpers
 */

#ifndef _H_MODEL_IO
    #define _H_MODEL_IO
    #include "models.h"
    #include "../util.h"


// Begin header "model io" ---------------------------------------------------------------------------------------------

/*
 * Identifies a serialized GradeBook when found in the first 4 bytes
 *  of a stream/file/array/filing cabinet in the basement of the city planning office
 *
 * Implemented in model_io.c
 */
extern const byte GRADEBOOK_MAGIC[4];

typedef enum SerializationStatus {

    /*
     * The serialization function completed successfully
     */
    SUCCESS             = 0,

    /*
     * Unspecified failure
     */
    FAILURE             = 1,

    /*
     * The provided buffer was too small
     */
    SHORT_BUFFER        = 2,

    /*
     * No known magic was provided at the beginning of the serial data
     */
    BAD_MAGIC           = 3,

    /*
     * A Student or Course was found having or referencing a Course ID that is not in the GradeBook
     */
    ILLEGAL_COURSE_ID   = 4,

    /*
     * A Student or Course was found having or referencing a Student ID that is not in the GradeBook
     */
    ILLEGAL_STUDENT_ID  = 5

} SerializationStatus;

/**
* Write the GradeBook referenced by pointer gradeBook to buffer, where buffer is GRADEBOOK_LENGTH bytes long
*/
SerializationStatus GradeBook_serialize(GradeBook* gradeBook, byte* buffer);

/**
* Construct necessary elements and create a GradeBook using the stream 'serialData'.
* Assign the constructed GradeBook to target.
*
* A return value of '0'
*/
SerializationStatus GradeBook_deserialize(byte* serialData, GradeBook* destination);

/*
 * Calculate the actual serialized size of a Student
 */
size sizeOfStudent(Student* student);

/*
 * Calculate the actual serialized size of a Course
 */
size sizeOfCourse(Course* course);

/*
 * Calculate the actual serialized size of a GradeBook
 * Note that this will return the size of the gradebook's students and courses as well.
 * For the size of the gradebook ONLY, see sizeOfGradeBookOnly()
 */
size sizeOfGradeBook(GradeBook* book);

/*
 * Calculate the size of a GradeBook excluding children
 */
size sizeOfGradeBookOnly(GradeBook* book);

// End Model "model_io" ------------------------------------------------------------------------------------------------

#endif