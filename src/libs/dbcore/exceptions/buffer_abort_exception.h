#ifndef __BUFFER_ABORT_EXCEPTION_H
#define __BUFFER_ABORT_EXCEPTION_H

#include <stdexcept>

namespace dbcore::exceptions
{
    class buffer_abort_exception : public std::runtime_error
    {
        public:
            buffer_abort_exception() : std::runtime_error("Buffer request could not be satisfied") {}
    };
}

#endif