#ifndef _H_COMMAND
    #define _H_COMMAND
    #include "../../models/models.h"

typedef enum E_ShellReturn {

    SR_SUCCESS  = 0x0,
    SR_FAILURE  = 0x1,
    SR_EXIT     = 0x2,
    SR_SAVE     = 0x3,
    SR_LOAD     = 0x4

} ShellReturn;

typedef ShellReturn(*ShellCommand)(char*, GradeBook* gradeBook);

#endif