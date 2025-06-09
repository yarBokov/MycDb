#ifndef __SSH_CLIENT_H
#define __SSH_CLIENT_H

#include <libssh/libssh.h>
#include <libssh/scp.h>
#include <libssh/callbacks.h>
#include <libssh/sftp.h>
#include <libssh/server.h>

#include "sessions.h"
#include "exceptions.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>

#include <fmt/fmt.h>

namespace ssh
{
    class ssh_client
    {
        private:
            ssh_session_ptr m_session;
            sftp_session_ptr m_sftp_session;
            std::mutex m_transfer_mtx;

        public:
            ssh_client()
                : m_session(ssh_new())
            {
                if (!m_session)
                    throw ssh_exception("Failed to create SSH session");

                ssh_options_set(m_session.get(), SSH_OPTIONS_STRICTHOSTKEYCHECK, "1");
            }

            ~ssh_client()
            {
                disconnect();
            }

            void connect(const std::string& host, int port = 22, int timeout)
            {
                ssh_options_set(m_session.get(), SSH_OPTIONS_HOST, host.c_str());
                ssh_options_set(m_session.get(), SSH_OPTIONS_PORT, &port);
                ssh_options_set(m_session.get(), SSH_OPTIONS_TIMEOUT, &timeout);

                int return_code = ssh_connect(m_session.get());
                if (return_code != SSH_OK)
                    throw ssh_exception(fmt::format("Connection to {}, port {} failed", host, port), m_session.get());
            }

            void disconnect()
            {
                if (m_sftp_session)
                    m_sftp_session.reset();

                if (m_session && ssh_is_connected(m_session.get()))
                    ssh_disconnect(m_session.get());
            }

            void set_timeout(int timeout)
            {
                ssh_options_set(m_session.get(), SSH_OPTIONS_TIMEOUT, &timeout);
            }

            void verify_host(bool strict = true)
            {
                if (!strict)
                {
                    ssh_options_set(m_session.get(), SSH_OPTIONS_STRICTHOSTKEYCHECK, "0");
                    return;
                }

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

            void auth_public_key(const std::string& username, const std::string& private_key_path = "", const std::string& password)
            {
                int return_code = 0;
                if (private_key_path.empty())
                {
                    return_code = ssh_userauth_publickey_auto(
                        m_session.get(), username.c_str(), password.empty() ? nullptr : password.c_str());
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
                std::scoped_lock<std::mutex> lock(m_transfer_mtx);

                std::ifstream file(local_path, std::ios::binary | std::ios::ate);
                if (!file)
                    throw scp_exception("Failed to open local file " + local_path);

                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);

                auto scp = ssh_scp_new(m_session.get(), SSH_SCP_WRITE, remote_path.c_str());
                if (!scp)
                    throw scp_exception("Failed to create session", m_session.get());

                std::unique_ptr<ssh_scp_struct, decltype(&ssh_scp_free)> scp_guard(scp, ssh_scp_free);

                int return_code = ssh_scp_init(scp);
                if (return_code != SSH_OK)
                {
                    throw scp_exception("Failed to initialize session", m_session.get());
                }
                
                return_code = ssh_scp_push_file(scp, remote_path.c_str(), size, mode);
                if (return_code != SSH_OK)
                {
                    throw scp_exception("Failed to create remote file " + remote_path, m_session.get());
                }

                std::vector<char> buffer(size);

                while (file)
                {
                    file.read(buffer.data(), buffer.size());
                    std::streamsize bytes_read = file.gcount();

                    if (bytes_read > 0)
                    {
                        return_code = ssh_scp_write(scp, buffer.data(), bytes_read);
                        if (return_code != SSH_OK)
                            throw scp_exception("Failed to write to remote file " + remote_path, m_session.get());
                        
                    }
                }

                ssh_scp_close(scp);
            }

            void scp_download(const std::string& local_path, const std::string& remote_path)
            {
                std::scoped_lock<std::mutex> lock(m_transfer_mtx);

                auto scp = ssh_scp_new(m_session.get(), SSH_SCP_READ, remote_path.c_str());
                if (!scp)
                    throw scp_exception("Failed to create session", m_session.get());

                std::unique_ptr<ssh_scp_struct, decltype(&ssh_scp_free)> scp_guard(scp, ssh_scp_free);

                int return_code = ssh_scp_init(scp);
                if (return_code != SSH_OK)
                {
                    throw scp_exception("Failed to initialize session", m_session.get());
                }
                
                return_code = ssh_scp_pull_request(scp);
                if (return_code != SSH_SCP_REQUEST_NEWFILE)
                {
                    throw scp_exception("Caught unexpected response while accessing remote file " + remote_path, m_session.get());
                }

                std::size_t size = ssh_scp_request_get_size(scp);
                std::ofstream file(local_path, std::ios::binary);
                if (!file)
                {
                    ssh_scp_free(scp);
                    throw std::runtime_error("Failed to open local file " + local_path + " for writing");
                }

                ssh_scp_accept_request(scp);

                std::vector<char> buffer(size);

                while(true)
                {
                    int bytes_read = ssh_scp_read(scp, buffer.data(), buffer.size());
                    if (bytes_read == SSH_ERROR)
                        throw scp_exception("Failed to read from remote file " + remote_path, m_session.get());

                    if (bytes_read == 0)
                        break;
                    
                    file.write(buffer.data(), bytes_read);
                    if (!file)
                        throw std::runtime_error("Failed to write to local file " + local_path);
                    
                }

                ssh_scp_close(scp);
            }

            void init_sftp()
            {
                if (m_sftp_session)
                    return;

                m_sftp_session = sftp_session_ptr(sftp_new(m_session.get()));
                if (!m_sftp_session)
                    throw sftp_exception("Failed to create SFTP session", m_session.get());

                int return_code = sftp_init(m_sftp_session.get());
                if (return_code != SSH_OK)
                    throw sftp_exception("Failed to initialize SFTP session", m_session.get());
            }

            void sftp_upload(const std::string& local_path, const std::string& remote_path, int mode = S_IRWXU)
            {
                std::scoped_lock<std::mutex> lock(m_transfer_mtx);

                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                std::ifstream file(local_path, std::ios::binary);
                if (!file)
                    throw std::runtime_error("Failed to open local file " + local_path);

                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);

                sftp_file remote_file = sftp_open(m_sftp_session.get(), remote_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
                if (!remote_file)
                    throw sftp_exception("Failed to open remote file " + remote_path, m_session.get());

                std::unique_ptr<sftp_file_struct, decltype(&sftp_close)> file_guard(remote_file, sftp_close);

                std::vector<char> buffer(4096);

                while (file.read(buffer.data(), buffer.size()))
                {
                    ssize_t bytes_written = sftp_write(remote_file, buffer.data(), file.gcount());
                    if (bytes_written != file.gcount())
                        throw sftp_exception("Failed to write to remote file", m_session.get());
                }
            }

            void sftp_download(const std::string& remote_path, const std::string& local_path)
            {
                std::scoped_lock<std::mutex> lock(m_transfer_mtx);

                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                sftp_file remote_file = sftp_open(m_sftp_session.get(), remote_path.c_str(), O_RDONLY, 0);
                if (!remote_file)
                    throw sftp_exception("Failed to open remote file " + remote_path, m_session.get());

                std::unique_ptr<sftp_file_struct, decltype(&sftp_close)> file_guard(remote_file, sftp_close);

                sftp_attributes attrs = sftp_fstat(remote_file);
                if (!attrs)
                    throw sftp_exception("Failed to get file attributes for " + remote_path, m_session.get());

                std::unique_ptr<sftp_attributes_struct, decltype(&sftp_attributes_free)> attrs_guard(attrs, sftp_attributes_free);
                uint64_t size = attrs->size;

                std::ofstream file(local_path, std::ios::binary);
                if (!file)
                {
                    throw std::runtime_error("Failed to open local file " + local_path);
                }

                std::vector<char> buffer(4096);
                
                while (true)
                {
                    ssize_t bytes_read = sftp_read(remote_file, buffer.data(), buffer.size());
                    if (bytes_read == 0) break;
                    if (bytes_read < 0)
                        throw sftp_exception("Failed to read from remote file " + remote_path, m_session.get());
                    
                    file.write(buffer.data(), bytes_read);
                    if (!file)
                        throw std::runtime_error("Failed to write to local file " + local_path);
                }
            }

            std::vector<std::string> sftp_list_directory(const std::string& remote_path)
            {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                sftp_dir dir = sftp_opendir(m_sftp_session.get(), remote_path.c_str());
                if (!dir)
                    throw sftp_exception("Failed to open remote directory " + remote_path, m_session.get());

                std::unique_ptr<sftp_dir_struct, decltype(&sftp_closedir)> dir_guard(dir, sftp_closedir);

                std::vector<std::string> result;
                sftp_attributes attributes;
                
                while ((attributes = sftp_readdir(m_sftp_session.get(), dir)) != nullptr)
                {
                    std::unique_ptr<sftp_attributes_struct, decltype(&sftp_attributes_free)> 
                        attrs_guard(attributes, sftp_attributes_free);
                    
                    if (strcmp(attributes->name, ".") != 0 && strcmp(attributes->name, "..") != 0)
                        result.emplace_back(attributes->name);
                }

                return result;
            }

            void sftp_create_directory(const std::string& remote_path, int mode = S_IRWXU)
            {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                int return_code = sftp_mkdir(m_sftp_session.get(), remote_path.c_str(), mode);
                if (return_code != SSH_OK)
                    throw sftp_exception("Failed to create remote directory " + remote_path, m_session.get());
            }

            void sftp_remove_directory(const std::string& remote_path)
            {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                int return_code = sftp_rmdir(m_sftp_session.get(), remote_path.c_str());
                if (return_code != SSH_OK)
                    throw sftp_exception("Failed to remove remote directory " + remote_path, m_session.get());
            }

            void sftp_delete_file(const std::string& remote_path)
            {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                int return_code = sftp_unlink(m_sftp_session.get(), remote_path.c_str());
                if (return_code != SSH_OK)
                    throw sftp_exception("Failed to delete remote file " + remote_path, m_session.get());
            }

            void sftp_rename(const std::string& old_path, const std::string& new_path)
            {
                if (!m_sftp_session)
                    throw sftp_exception("SFTP session not initialized");

                sftp_rename(old_path.c_str(), new_path.c_str());
            }

            std::string execute_command(const std::string& command, int timeout = 0)
            {
                std::string error = "Command execution failed: {}";
                ssh_channel channel = ssh_channel_new(m_session.get());
                if (!channel)
                    throw ssh_exception(fmt::format(error, "Failed to create SSH channel"), m_session.get());

                std::unique_ptr<ssh_channel_struct, decltype(&ssh_channel_free)> channel_guard(channel, ssh_channel_free);

                int return_code = ssh_channel_open_session(channel);
                if (return_code != SSH_OK)
                {
                    throw ssh_exception(fmt::format(error, "Failed to open SSH channel"), m_session.get());
                }

                if (timeout > 0)
                    ssh_channel_set_blocking(channel, 0);

                return_code = ssh_channel_request_exec(channel, command.c_str());
                if (return_code != SSH_OK)
                {
                    throw ssh_exception(fmt::format(error, "SSH channel cannot execute command"), m_session.get());
                }

                std::string result;
                char buf[1024];
                int nbytes = 0;
                bool eof = false;
                auto start_time = std::chrono::steady_clock::now();

                while (!eof) {
                    if (timeout > 0) {
                        auto current_time = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
                        
                        if (elapsed >= timeout)
                        {
                            ssh_channel_close(channel);
                            throw ssh_exception("Command execution timed out: " + command, m_session.get());
                        }

                        struct timeval tv;
                        tv.tv_sec = (timeout - elapsed) / 1000;
                        tv.tv_usec = ((timeout - elapsed) % 1000) * 1000;

                        ssh_channel_poll(channel, 0);
                    }

                    nbytes = ssh_channel_read(channel, buf, sizeof(buf), 0);
                    if (nbytes > 0)
                    {
                        result.append(buf, nbytes);
                    } 
                    else if (nbytes == 0)
                    {
                        eof = ssh_channel_is_eof(channel);
                        if (!eof)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                    }
                    else 
                    {
                        throw ssh_exception("Error reading from SSH channel", m_session.get());
                    }
                }

                int exit_status = ssh_channel_get_exit_status(channel);
                if (exit_status != 0)
                {
                    throw ssh_exception(fmt::format("Command failed with exit status {}: {}", exit_status, command), m_session.get());
                }

                return result;
            }

            std::pair<std::string, std::string> execute_command_with_stderr(const std::string& command, int timeout = 0)
            {
                ssh_channel channel = ssh_channel_new(m_session.get());
                if (!channel)
                    throw ssh_exception("Failed to create SSH channel", m_session.get());

                std::unique_ptr<ssh_channel_struct, decltype(&ssh_channel_free)> channel_guard(channel, ssh_channel_free);

                int return_code = ssh_channel_open_session(channel);
                if (return_code != SSH_OK)
                    throw ssh_exception("Failed to open SSH channel", m_session.get());

                return_code = ssh_channel_request_pty(channel);
                if (return_code != SSH_OK)
                    throw ssh_exception("Failed to request PTY for command: " + command, m_session.get());

                return_code = ssh_channel_request_shell(channel);
                if (return_code != SSH_OK)
                    throw ssh_exception("Failed to request shell for command: " + command, m_session.get());

                std::string cmd_with_newline = command + "\n";
                ssh_channel_write(channel, cmd_with_newline.c_str(), cmd_with_newline.size());

                std::string stdout_result;
                std::string stderr_result;
                char buffer[1024];
                bool eof = false;
                auto start_time = std::chrono::steady_clock::now();

                while (!eof)
                {
                    if (timeout > 0)
                    {
                        auto current_time = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
                        
                        if (elapsed >= timeout) {
                            ssh_channel_close(channel);
                            throw ssh_exception("Command execution timed out: " + command, m_session.get());
                        }
                    }

                    int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
                    if (nbytes > 0)
                        stdout_result.append(buffer, nbytes);
                    else if (nbytes == SSH_ERROR)
                        throw ssh_exception("Error reading from SSH channel stdout", m_session.get());
                    

                    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 1);
                    if (nbytes > 0)
                        stderr_result.append(buffer, nbytes);
                    else if (nbytes == SSH_ERROR)
                        throw ssh_exception("Error reading from SSH channel stderr", m_session.get());
                    

                    if (nbytes == 0)
                    {
                        eof = ssh_channel_is_eof(channel);
                        if (!eof)
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }

                int exit_status = ssh_channel_get_exit_status(channel);
                if (exit_status != 0)
                {
                    throw ssh_exception(fmt::format("Command failed with exit status {}: {}\nStderr: {}", 
                        exit_status, command, stderr_result), m_session.get());
                }

                return {stdout_result, stderr_result};
            }
    };
}

#endif