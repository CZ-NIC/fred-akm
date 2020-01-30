/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SUBPROCESS_HH_91D7AE444630995FCAD6ED285B6170FD//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define SUBPROCESS_HH_91D7AE444630995FCAD6ED285B6170FD

#include <map>
#include <array>
#include <unistd.h>
#include <sys/wait.h>


namespace Fred {
namespace Akm {


struct Descriptor
{
    enum Enum
    {
        READ = 0,
        WRITE = 1
    };
};

class Pipe
{
private:
    int fd_[2];

public:
    Pipe()
    {
        if (const int ret = pipe(fd_) != 0)
        {
            throw std::runtime_error("create of pipe failed (ret=" + std::to_string(ret) + ")");
        }
    }

    ~Pipe()
    {
        close();
    }

    int read_fd() const
    {
        return fd_[Descriptor::READ];
    }

    int write_fd() const
    {
        return fd_[Descriptor::WRITE];
    }

    void close()
    {
        ::close(fd_[Descriptor::READ]);
        ::close(fd_[Descriptor::WRITE]);
    }
};


class Subprocess
{
private:
    Pipe write_pipe_;
    Pipe read_pipe_;
    pid_t parent_pid_ = -1;
    pid_t child_pid_ = -1;

public:
    Subprocess(const std::vector<const char*>& _argv)
    {
        parent_pid_ = ::getpid();
        child_pid_ = ::fork();

        if (child_pid_ == -1)
        {
            throw std::runtime_error("running subprocess failed (fork() failed)");
        }
        else if (child_pid_ == 0)
        {
            /* child */
            ::dup2(write_pipe_.read_fd(), STDIN_FILENO);
            ::dup2(read_pipe_.write_fd(), STDOUT_FILENO);

            write_pipe_.close();
            read_pipe_.close();

            ::execv(_argv[0], const_cast<char **>(&_argv[0]));
            throw std::runtime_error("subprocess call failed");
        }
        else
        {
            ::close(write_pipe_.read_fd());
            ::close(read_pipe_.write_fd());
        }
    }

    pid_t parent_pid() const
    {
        return parent_pid_;
    }

    pid_t child_pid() const
    {
        return child_pid_;
    }

    int wait()
    {
        int child_status;
        const pid_t exited_child = waitpid(child_pid_, &child_status, 0);
        if (exited_child == child_pid_)
        {
            const bool child_exited_successfully = WIFEXITED(child_status) && (WEXITSTATUS(child_status) == EXIT_SUCCESS);
            if (!child_exited_successfully)
            {
                throw std::runtime_error("child terminated unsuccesfully :(");
            }
        }
        else
        {
            throw std::runtime_error("waitpid failure (" + std::string(std::strerror(errno)) + ")");
        }
        return WEXITSTATUS(child_status);
    }

    ssize_t write(const std::string& _buffer)
    {
        if (_buffer.size() > 0)
        {
            const ssize_t ret = ::write(write_pipe_.write_fd(), _buffer.c_str(), _buffer.size());
            if (ret == -1)
            {
                throw std::runtime_error("write to pipe failed");
            }
            else if (static_cast<size_t>(ret) != _buffer.size())
            {
                throw std::runtime_error("write to pipe incomplete?");
            }
            return ret;
        }
        return 0;
    }

    void write_eof()
    {
        ::close(write_pipe_.write_fd());
    }

    typedef std::array<char, 1024> ReadBuffer;

    int read(ReadBuffer& _buffer)
    {
        return ::read(read_pipe_.read_fd(), _buffer.data(), _buffer.size() - 1);
    }

};


} // namespace Akm
} // namespace Fred

#endif//SUBPROCESS_HH_91D7AE444630995FCAD6ED285B6170FD
