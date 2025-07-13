#ifndef __SERVER_CORE_EXCEPTIONS_H
#define __SERVER_CORE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

#include <fmt/format.h>

namespace server::core
{
    class conf_editor_exception : std::runtime_error
    {
        public:
            conf_editor_exception(const std::string& msg):
                std::runtime_error(fmt::format("CONF_EDITOR error: {}", msg))
            {}
    };
}

#endif