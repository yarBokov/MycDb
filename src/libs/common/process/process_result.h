#ifndef __PROCESS_RESULT_STRUCT_H
#define __PROCESS_RESULT_STRUCT_H

#include <string>

namespace common::process
{
    struct process_result
    {
        int exit_code;
        std::string output;
        std::string error;
    };
}

#endif