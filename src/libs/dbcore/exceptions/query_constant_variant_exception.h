#ifndef __QUERY_CONSTANT_VARIANT_EXCEPTION_H
#define __QUERY_CONSTANT_VARIANT_EXCEPTION_H

#include <stdexcept>

namespace dbcore::exceptions
{
    class query_constant_variant_exception : public std::runtime_error
    {
        public:
            query_constant_variant_exception(std::string err) 
                : std::runtime_error("Query constant variant error: " + err) 
            {}
    };
}

#endif