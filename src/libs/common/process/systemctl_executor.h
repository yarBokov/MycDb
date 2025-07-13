#ifndef __PROCESS_SYSTEMCTL_EXECUTOR_H
#define __PROCESS_SYSTEMCTL_EXECUTOR_H

#include "process_executor.h"

namespace common::process
{
    class systemctl_process_executor
    {
        public:
            systemctl_process_executor() = delete;

            static process_result restart(const std::string& service)
            {
                return process_executor::execute("systemctl", { "restart", service });
            }

            static process_result start(const std::string& service)
            {
                return process_executor::execute("systemctl", { "start", service });
            }

            static process_result enable(const std::string& service)
            {
                return process_executor::execute("systemctl", { "enable", service });
            }

            static process_result stop(const std::string& service)
            {
                return process_executor::execute("systemctl", { "stop", service });
            }

            static process_result status(const std::string& service)
            {
                return process_executor::execute("systemctl", { "status", service });
            }
    };
}

#endif