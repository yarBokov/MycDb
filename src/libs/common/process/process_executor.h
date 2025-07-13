#ifndef __PROCESS_EXECUTOR_H
#define __PROCESS_EXECUTOR_H

#include "process_result.h"

#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <thread>
#include <mutex>
#include <system_error>
#include <stdexcept>
#include <cstring>
#include <array>
#include <memory>
#include <fmt/format.h>

namespace common::process
{
    class process_executor
    {
        public:
            process_executor() = delete;

            static process_result execute(const std::string& command, const std::vector<std::string>& args = {})
            {
                std::vector<const char*> argv;
                argv.reserve(args.size() + 2);
                argv.push_back(command.c_str());
                for (const auto& arg : args)
                {
                    argv.push_back(arg.c_str());
                }
                argv.push_back(nullptr);

                std::array<int, 2> stdout_pipe{};
                std::array<int, 2> stderr_pipe{};

                if (pipe(stdout_pipe.data()))
                    throw std::system_error(errno, std::generic_category(), "pipe stdout failed, process_executor");
                if (pipe(stderr_pipe.data()))
                {
                    close(stdout_pipe[0]);
                    close(stdout_pipe[1]);
                    throw std::system_error(errno, std::generic_category(), "pipe stdout failed, process_executor");
                }

                pid_t pid = fork();
                if (pid < 0)
                {
                    close_pipes(stdout_pipe, stderr_pipe);
                    throw std::system_error(errno, std::generic_category(), "fork failed, process_executor");
                }

                if (pid == 0)
                {
                    close(stdout_pipe[0]);
                    close(stderr_pipe[0]);

                    dup2(stdout_pipe[1], STDOUT_FILENO);
                    dup2(stderr_pipe[1], STDOUT_FILENO);
                    close(stdout_pipe[1]);
                    close(stderr_pipe[1]);

                    execvp(command.c_str(), const_cast<char* const*>(argv.data()));

                    _exit(EXIT_FAILURE);
                }

                close(stdout_pipe[1]);
                close(stderr_pipe[1]);

                std::string stdout_output, stderr_output;
                std::mutex output_mtx;
                bool threads_active = true;

                auto reader = [](int fd, std::string& output, std::mutex& mtx, bool& active)
                {
                    char buffer[4096];
                    ssize_t count;
                    while (active)
                    {
                        count = read(fd, buffer, sizeof(buffer));
                        if (count > 0)
                        {
                            std::lock_guard lock(mtx);
                            output.append(buffer, count);
                        }
                        else if (count == 0 || (!active && count == -1))
                            break;
                        else if (errno != EINTR)
                            break;
                    }
                };

                std::thread stdout_thr(reader, stdout_pipe[0], std::ref(stdout_output),
                                       std::ref(output_mtx), std::ref(threads_active));

                std::thread stderr_thr(reader, stderr_pipe[0], std::ref(stderr_output),
                                       std::ref(output_mtx), std::ref(threads_active));

                int status;
                waitpid(pid, &status, 0);
                int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

                {
                    std::lock_guard lock(output_mtx);
                    threads_active = false;
                }

                close(stdout_pipe[0]);
                close(stderr_pipe[0]);

                stdout_thr.join();
                stderr_thr.join();

                return {exit_code, stdout_output, stderr_output};
            }

        private:
            static void close_pipes(const std::array<int, 2>& p1, const std::array<int, 2>& p2)
            {
                close(p1[0]);
                close(p1[1]);
                close(p2[0]);
                close(p2[1]);
            }
    };
}

#endif