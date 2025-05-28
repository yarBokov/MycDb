#ifndef __LOCK_ABORT_EXCEPTION_H
#define __LOCK_ABORT_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace dbcore::exceptions
{
    class lock_abort_exception : public std::runtime_error
    {
        public:
            lock_abort_exception(std::string err) : std::runtime_error(err + " request could not be satisfied") {}
    };
}

#endif