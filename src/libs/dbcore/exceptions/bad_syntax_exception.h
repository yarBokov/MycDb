#ifndef __BAD_SYNTAX_EXCEPTION_H
#define __BAD_SYNTAX_EXCEPTION_H

#include <stdexcept>

namespace dbcore::exceptions
{
    class bad_syntax_exception : public std::runtime_error
    {
        public:
            bad_syntax_exception()
                : std::runtime_error("Bad SQL query syntax")
            {}
    };
}

#endif
