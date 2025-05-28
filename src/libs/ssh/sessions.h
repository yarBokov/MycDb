#ifndef __SSH_SESSIONS_H
#define __SSH_SESSIONS_H

#include <libssh/session.h>
#include <libssh/sftp.h>

#include <memory>

namespace ssh
{
    struct ssh_session_deallocator
    {
        void operator()(ssh_session session) const
        {
            ssh_disconnect(session);
            ssh_free(session);
        }
    };

    using ssh_session_ptr = std::unique_ptr<ssh_session_struct, ssh_session_deallocator>;

    struct sftp_session_deallocator
    {
        void operator()(sftp_session session) const
        {
            sftp_free(session);
        }
    };

    using sftp_session_ptr = std::unique_ptr<sftp_session_struct, sftp_session_deallocator>;
}

#endif