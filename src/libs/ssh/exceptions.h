#ifndef __SSH_EXCEPTION_H
#define __SSH_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <libssh/session.h>

namespace ssh
{
    class ssh_exception : public std::runtime_error
    {
        public:
            ssh_exception(const std::string& msg, ssh_session session = nullptr)
                : std::runtime_error(msg + (session ? (": " + std::string(ssh_get_error(session))) :
                ""))
            {}
    };

    class scp_exception : public std::runtime_error
    {
        public:
            scp_exception(const std::string& msg, ssh_session session = nullptr)
                : std::runtime_error("SCP operation error: " + msg + 
                    (session ? (": " + std::string(ssh_get_error(session))) : ""))
            {}
    };

    class sftp_exception : public std::runtime_error
    {
        public:
            sftp_exception(const std::string& msg, ssh_session session = nullptr)
                : std::runtime_error("SFTP operation error: " + msg + 
                    (session ? (": " + std::string(ssh_get_error(session))) : ""))
            {}
    };
}

#endif