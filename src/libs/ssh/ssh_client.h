#ifndef __SSH_CLIENT_H
#define __SSH_CLIENT_H

#include <libssh/libssh.h>
#include <libssh/scp.h>

#include "sessions.h"
#include "exceptions.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <vector>

#include <fmt/fmt.h>

namespace ssh
{
    class ssh_client
    {
        private:
            ssh_session_ptr m_session;
            sftp_session_ptr m_sftp_session;

        public:
            ssh_client()
                : m_session(ssh_new())
            {
                if (!m_session)
                    throw ssh_exception("Failed to create SSH session");
            }

            ~ssh_client() = default;

            void connect(const std::string& host, int port = 22)
            {
                ssh_options_set(m_session.get(), SSH_OPTIONS_HOST, host.c_str());
                ssh_options_set(m_session.get(), SSH_OPTIONS_PORT, &port);

                int return_code = ssh_connect(m_session.get());
                if (return_code != SSH_OK)
                    throw ssh_exception(fmt::format("Connection to {}, port {} failed", host, port), m_session.get());
            }

            void verify_host()
            {
                int state = ssh_is_server_known(m_session.get());
                if (state == state == SSH_SERVER_KNOWN_CHANGED || 
                    state == SSH_SERVER_FOUND_OTHER ||
                    state == SSH_SERVER_FILE_NOT_FOUND || 
                    state == SSH_SERVER_NOT_KNOWN)
                {
                    unsigned char* hash = nullptr;

                    if (ssh_get_pubkey_hash(m_session.get(), &hash) != 0)
                        throw ssh_exception("Failed to get host key hash", m_session.get());

                    std::unique_ptr<unsigned char, decltype(&free)> hash_ptr(hash, free);

                    if (ssh_write_knownhost(m_session.get()) < 0)
                        throw ssh_exception("Failed to save host key", m_session.get());
                }
                else if (state != SSH_SERVER_KNOWN_OK)
                    throw ssh_exception("Host verification failed", m_session.get());
            }

            void auth_password(const std::string& username, const std::string& password)
            {
                int return_code = ssh_userauth_password(m_session.get(), username.c_str(), password.c_str());
                if (return_code != SSH_AUTH_SUCCESS)
                    throw ssh_exception("Password authentication failed", m_session.get());
            }

            void auth_publickey(const std::string& username, const std::string& private_key_path = "")
            {
                int return_code = 0;
                if (private_key_path.empty())
                {
                    return_code = ssh_userauth_publickey_auto(
                        m_session.get(), username.c_str(), nullptr);
                }
                else
                {
                    return_code = ssh_userauth_publickey_auto(
                        m_session.get(), username.c_str(), private_key_path.c_str());
                }

                if (return_code != SSH_AUTH_SUCCESS)
                    throw ssh_exception("Public key authentication failed", m_session.get());
            }

            void scp_upload(const std::string& local_path, const std::string& remote_path, int mode = S_IRWXU)
            {
                std::ifstream file(local_path, std::ios::binary | std::ios::ate);
                if (!file)
                    throw scp_exception("Failed to open local file " + local_path);

                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);

                auto scp = ssh_scp_new(m_session.get(), SSH_SCP_WRITE, remote_path.c_str());
                if (!scp)
                    throw scp_exception("Failed to create session", m_session.get());

                int return_code = ssh_scp_init(scp);
                if (return_code != SSH_OK)
                {
                    ssh_scp_free(scp);
                    throw scp_exception("Failed to initialize session", m_session.get());
                }
                
                return_code = ssh_scp_push_file(scp, remote_path.c_str(), size, mode);
                if (return_code != SSH_OK) {
                    ssh_scp_free(scp);
                    throw scp_exception("Failed to create remote file " + remote_path, m_session.get());
                }

                std::vector<char> buffer(size);
                if (!file.read(buffer.data(), size))
                {
                    ssh_scp_free(scp);
                    throw std::runtime_error("Failed to read local file " + local_path + " in buffer");
                }

                return_code = ssh_scp_write(scp, buffer.data(), size);
                if (return_code != SSH_OK)
                {
                    ssh_scp_free(scp);
                    throw scp_exception("Failed to write to remote file " + remote_path, m_session.get());;
                }

                ssh_scp_close(scp);
                ssh_scp_free(scp);
            }

            void scp_download(const std::string& local_path, const std::string& remote_path)
            {
                auto scp = ssh_scp_new(m_session.get(), SSH_SCP_READ, remote_path.c_str());
                if (!scp)
                    throw scp_exception("Failed to create session", m_session.get());

                int return_code = ssh_scp_init(scp);
                if (return_code != SSH_OK)
                {
                    ssh_scp_free(scp);
                    throw scp_exception("Failed to initialize session", m_session.get());
                }
                
                return_code = ssh_scp_pull_request(scp);
                if (return_code != SSH_SCP_REQUEST_NEWFILE) {
                    ssh_scp_free(scp);
                    throw scp_exception("Caught unexpected response while accessing remote file " + remote_path, m_session.get());
                }

                std::size_t size = ssh_scp_request_get_size(scp);
                std::vector<char> buffer(size);

                std::ofstream file(local_path, std::ios::binary);
                if (!file)
                {
                    ssh_scp_free(scp);
                    throw std::runtime_error("Failed to open local file " + local_path + " for writing");
                }

                ssh_scp_accept_request(scp);

                std::size_t received = 0;
                std::size_t nbytes = 0;
                while(received < size)
                {
                    nbytes = ssh_scp_read(scp, buffer.data() + received, size - received);
                    if (nbytes == SSH_ERROR)
                    {
                        ssh_scp_free(scp);
                        throw scp_exception(fmt::format("Failed to read from remote file {} to local {}", remote_path, local_path), 
                            m_session.get());                       
                    }
                    received += nbytes;
                }

                ssh_scp_close(scp);
                ssh_scp_free(scp);
            }

            void init_sftp() {
                m_sftp_session = sftp_session_ptr(sftp_new(m_session.get()));
                if (!m_sftp_session)
                    throw sftp_exception("Failed to create SFTP session", m_session.get());

                int return_code = sftp_init(m_sftp_session.get());
                if (return_code != SSH_OK)
                    throw sftp_exception("Failed to initialize SFTP session", m_session.get());
            }

            void sftp_upload(const std::string& local_path, const std::string& remote_path, int mode = S_IRWXU) {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                std::ifstream file(local_path, std::ios::binary);
                if (!file)
                    throw std::runtime_error("Failed to open local file " + local_path);

                sftp_file remote_file = sftp_open(m_sftp_session.get(), remote_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
                if (!remote_file)
                    throw sftp_exception("Failed to open remote file " + remote_path, m_session.get());

                char buffer[4096];
                while (file.read(buffer, sizeof(buffer)))
                {
                    ssize_t bytes_written = sftp_write(remote_file, buffer, file.gcount());
                    if (bytes_written != file.gcount())
                    {
                        sftp_close(remote_file);
                        throw sftp_exception("Failed to write to remote file", m_session.get());
                    }
                }

                sftp_close(remote_file);
            }

            void sftp_download(const std::string& remote_path, const std::string& local_path) {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                sftp_file remote_file = sftp_open(m_sftp_session.get(), remote_path.c_str(), O_RDONLY, 0);
                if (!remote_file)
                    throw sftp_exception("Failed to open remote file " + remote_path, m_session.get());

                std::ofstream file(local_path, std::ios::binary);
                if (!file)
                {
                    sftp_close(remote_file);
                    throw std::runtime_error("Failed to open local file " + local_path);
                }

                char buffer[4096];
                ssize_t bytes_read;
                while ((bytes_read = sftp_read(remote_file, buffer, sizeof(buffer))) > 0)
                {
                    file.write(buffer, bytes_read);
                    if (!file)
                    {
                        sftp_close(remote_file);
                        throw std::runtime_error(fmt::format("Failed to write to local file {} from remote {}", local_path, remote_path));
                    }
                }

                if (bytes_read < 0)
                {
                    sftp_close(remote_file);
                    throw sftp_exception("Failed to read from remote file " + remote_path, m_session.get());
                }

                sftp_close(remote_file);
            }

            std::string execute_command(const std::string& command)
            {
                std::string error = "Command execution failed: {}";
                ssh_channel channel = ssh_channel_new(m_session.get());
                if (!channel)
                    throw ssh_exception(fmt::format(error, "Failed to create SSH channel"), m_session.get());

                int return_code = ssh_channel_open_session(channel);
                if (return_code != SSH_OK)
                {
                    ssh_channel_free(channel);
                    throw ssh_exception(fmt::format(error, "Failed to open SSH channel"), m_session.get());
                }

                return_code = ssh_channel_request_exec(channel, command.c_str());
                if (return_code != SSH_OK)
                {
                    ssh_channel_close(channel);
                    ssh_channel_free(channel);
                    throw ssh_exception(fmt::format(error, "SSH channel cannot execute command"), m_session.get());
                }

                std::string result;
                char buf[1024];
                int nbytes = 0;
                while ((nbytes == ssh_channel_read(channel, buf, sizeof(buf), 0)) > 0)
                    result.append(buf, nbytes);

                if (nbytes < 0)
                {
                    ssh_channel_close(channel);
                    ssh_channel_free(channel);
                    throw ssh_exception(fmt::format(error, "SSH channel cannot be read"), m_session.get());
                }

                ssh_channel_send_eof(channel);
                ssh_channel_close(channel);
                ssh_channel_free(channel);

                return result;
            }
    };
}

#endif