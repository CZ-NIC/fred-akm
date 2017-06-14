#include <array>
#include <iostream>
#include <unistd.h>

#include "src/utils.hh"
#include "src/external_scanner.hh"

namespace Fred {
namespace Akm {


ExternalScannerTool::ExternalScannerTool(const std::string& _external_tool_path)
    : external_tool_path_(split_on(_external_tool_path, ' '))
{
    for (const auto& path_part : external_tool_path_)
    {
        /* path_part.c_str() is owned by external_tool_path_ vector */
        subprocess_argv_.push_back(path_part.c_str());
    }
    subprocess_argv_.push_back(nullptr);
    if (subprocess_argv_.size() < 2)
    {
        throw std::runtime_error("no scanner tool path supplied");
    }
}


void ExternalScannerTool::add_tasks(std::vector<NameserverDomains>& _tasks)
{
    tasks_.reserve(tasks_.size() + _tasks.size());
    std::copy(_tasks.begin(), _tasks.end(), std::inserter(tasks_, tasks_.end()));
    _tasks.clear();
}


void ExternalScannerTool::scan(OnResultCallback _callback) const
{
    struct ParentPipe
    {
        enum Enum
        {
            WRITE = 0,
            READ = 1
        };
    };
    struct Descriptor
    {
        enum Enum
        {
            READ = 0,
            WRITE = 1
        };
    };

    int pipes[2][2];
    pipe(pipes[ParentPipe::READ]);
    pipe(pipes[ParentPipe::WRITE]);

    int& parent_rd_fd = pipes[ParentPipe::READ][Descriptor::READ];
    int& parent_wr_fd = pipes[ParentPipe::WRITE][Descriptor::WRITE];
    int& child_rd_fd = pipes[ParentPipe::WRITE][Descriptor::READ];
    int& child_wr_fd = pipes[ParentPipe::READ][Descriptor::WRITE];

    if (!fork())
    {
        /* child */
        dup2(child_rd_fd, STDIN_FILENO);
        dup2(child_wr_fd, STDOUT_FILENO);

        close(child_rd_fd);
        close(child_wr_fd);
        close(parent_rd_fd);
        close(parent_wr_fd);

        ::execv(subprocess_argv_[0], const_cast<char **>(&subprocess_argv_[0]));
        throw std::runtime_error("external scanner tool subprocess call failed");
    }
    else
    {
        close(child_rd_fd);
        close(child_wr_fd);

        long total_tasks = 0;
        for (const auto& nameserver_task : tasks_)
        {
            auto line = nameserver_task.nameserver;
            for (const auto& domain : nameserver_task.nameserver_domains)
            {
                line += " " + domain.fqdn;
                total_tasks += 1;
            }
            line += "\n";
            write(parent_wr_fd, line.c_str(), line.size());
        }
        close(parent_wr_fd);
        std::cout << "total tasks sent: " << total_tasks << std::endl;

        long total_results = 0;
        auto process_result = [&total_results](std::string& buffer)
        {
            const auto& newline_ptr = std::find(buffer.begin(), buffer.end(), '\n');
            if (newline_ptr != buffer.end())
            {
                const auto result_line = std::string(buffer.begin(), newline_ptr);
                buffer.erase(buffer.begin(), newline_ptr + 1);
                //std::cout << "line: " << result_line << std::endl;
                total_results += 1;
                return true;
            }
            return false;
        };

        std::array<char, 256> chunk;
        std::string buffer;
        int read_count = 0;

        while ((read_count = read(parent_rd_fd, chunk.data(), chunk.size() - 1)) > 0)
        {
            if (read_count >= 0)
            {
                chunk[read_count] = 0;
                //std::cout << read_count << ": " << chunk.data() << std::endl;
                buffer += std::string(chunk.data());
                chunk = {{}};
                while (process_result(buffer)) { }
            }
            else
            {
                throw std::runtime_error("IOError");
            }
        }
        std::cout << "rest of buffer: " << buffer << std::endl;
        std::cout << "total processed results: " << total_results << std::endl;
    }
}


} //namespace Akm
} //namespace Fred
