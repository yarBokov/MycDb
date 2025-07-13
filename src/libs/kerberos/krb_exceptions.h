#ifndef __KERBEROS_EXCEPTIONS_H
#define __KERBEROS_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace kerberos
{
    class krb_file_configuration_exception : public std::runtime_error
    {
        public:
            krb_file_configuration_exception(std::string err)
                : std::runtime_error("KERBEROS FILE CONFIGURATION error: " + err)
            {}
    };
}

#endif