/*
 * Roman Hargrave, 2015.
 * CSCE 1040. ***REMOVED***
 *
 * Provides necessary bits for defining shell commands externally.
 */

#ifndef _H_COMMAND
    #define _H_COMMAND
    #include "../../models/models.h"

/*
 * Shell return value.
 * This is returned by the functions to indicate success status to the shell.
 *
 * - SR_SUCCESS denotes that the call completed successfully.
 * - SR_FAILURE denotes that the call failed.
 *   The command should provide a description of what went wrong prior to returning this value.
 *
 * - SR_EXIT will cause the shell to save the working index and exit
 * - SR_SAVE will cause the shell to save the working index
 * - SR_LOAD will cause the shell to load the working index
 */
typedef enum E_ShellReturn {

    SR_SUCCESS  = 0x0,
    SR_FAILURE  = 0x1,
    SR_EXIT     = 0x2,
    SR_SAVE     = 0x3,
    SR_LOAD     = 0x4

} ShellReturn;

typedef ShellReturn(*ShellCommand)(char*, GradeBook* gradeBook);

#endif