#include <array>
#include <iostream>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

#include "src/utils.hh"
#include "src/external_scanner.hh"

namespace Fred {
namespace Akm {

namespace {

    ScanResult scan_result_from_string(const std::string& value)
    {
        std::vector<std::string> parsed;
        parsed.reserve(8);

        auto beg_ptr = value.begin();
        auto end_ptr = value.end();
        auto nxt_ptr = std::find(beg_ptr, end_ptr, ' ');
        while (nxt_ptr != end_ptr)
        {
            parsed.emplace_back(std::string(beg_ptr, nxt_ptr));
            beg_ptr = nxt_ptr + 1;
            nxt_ptr = std::find(beg_ptr, end_ptr, ' ');
        }
        parsed.emplace_back(beg_ptr, nxt_ptr);

        if (parsed[0] == "insecure")
        {
            ScanResult result;
            result.cdnskey_status = parsed[0];

            if (parsed.size() == 8)
            {
                result.nameserver = parsed[1];
                result.nameserver_ip = parsed[2];
                result.domain_name = parsed[3];
                result.cdnskey_flags = boost::lexical_cast<int>(parsed[4]);
                result.cdnskey_proto = boost::lexical_cast<int>(parsed[5]);
                result.cdnskey_alg = boost::lexical_cast<int>(parsed[6]);
                result.cdnskey_public_key = parsed[7];

                return result;
            }
            else
            {
                throw std::runtime_error("malformed scan result (" + value + ")");
            }
        }
        if (parsed[0] == "unresolved")
        {
            ScanResult result;
            result.cdnskey_status = parsed[0];

            if (parsed.size() == 4)
            {
                result.nameserver = parsed[1];
                result.nameserver_ip = parsed[2];
                result.domain_name = parsed[3];

                return result;
            }
            else
            {
                throw std::runtime_error("malformed scan result (" + value + ")");
            }
        }
        throw std::runtime_error("unimplemented scan result type (" + parsed[0] + ")");
    }

}

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


void ExternalScannerTool::scan(OnResultsCallback _callback) const
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

        /* TODO: to normal function? */
        auto buffer_results = [](std::string& _raw_buffer, std::vector<ScanResult>& _result_buffer)
        {
            const auto newline_ptr = std::find(_raw_buffer.begin(), _raw_buffer.end(), '\n');
            if (newline_ptr != _raw_buffer.end())
            {
                const auto result_line = std::string(_raw_buffer.begin(), newline_ptr);
                _raw_buffer.erase(_raw_buffer.begin(), newline_ptr + 1);
                try
                {
                    _result_buffer.emplace_back(scan_result_from_string(result_line));
                }
                catch (const std::exception& ex)
                {
                    std::cerr << ex.what() << std::endl;
                }
                return true;
            }
            return false;
        };

        long total_results = 0;
        std::vector<ScanResult> result_buffer;
        result_buffer.reserve(1024);

        std::array<char, 512> chunk;
        //std::vector<char> raw_buffer(1024);
        std::string raw_buffer;
        int read_count = 0;

        while ((read_count = read(parent_rd_fd, chunk.data(), chunk.size() - 1)) > 0)
        {
            if (read_count >= 0)
            {
                chunk[read_count] = 0;
                raw_buffer += std::string(chunk.data());
                chunk = {{}};
                while (buffer_results(raw_buffer, result_buffer))
                {
                    if (result_buffer.size() == result_buffer.capacity())
                    {
                        _callback(result_buffer);
                        total_results += result_buffer.size();
                        result_buffer.clear();
                    }
                }
            }
            else
            {
                throw std::runtime_error("IOError");
            }
        }
        if (result_buffer.size())
        {
            _callback(result_buffer);
        }
        std::cout << "rest of buffer: " << raw_buffer.data() << std::endl;
        std::cout << "total processed results: " << total_results << std::endl;
    }
}


} //namespace Akm
} //namespace Fred
