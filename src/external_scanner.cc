#include <map>
#include <array>
#include <iostream>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

#include "src/utils.hh"
#include "src/external_scanner.hh"

namespace Fred {
namespace Akm {

namespace {

    class ScanResultParser
    {
    private:
        typedef std::vector<std::string> Tokens;

        ScanResult parse_type_insecure(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_INSECURE;

            result.nameserver = _tokens.at(1);
            result.nameserver_ip = _tokens.at(2);
            result.domain_name = _tokens.at(3);
            result.cdnskey_flags = boost::lexical_cast<int>(_tokens.at(4));
            result.cdnskey_proto = boost::lexical_cast<int>(_tokens.at(5));
            result.cdnskey_alg = boost::lexical_cast<int>(_tokens.at(6));
            result.cdnskey_public_key = _tokens.at(7);

            return result;
        }

        ScanResult parse_type_unresolved(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_UNRESOLVED;

            result.nameserver = _tokens.at(1);
            result.nameserver_ip = _tokens.at(2);
            result.domain_name = _tokens.at(3);

            return result;
        }

        ScanResult parse_type_secure(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_SECURE;

            result.domain_name = _tokens.at(1);
            result.cdnskey_flags = boost::lexical_cast<int>(_tokens.at(2));
            result.cdnskey_proto = boost::lexical_cast<int>(_tokens.at(3));
            result.cdnskey_alg = boost::lexical_cast<int>(_tokens.at(4));
            result.cdnskey_public_key = _tokens.at(5);

            return result;
        }

        ScanResult parse_type_untrustworthy(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_UNTRUSTWORTHY;

            result.domain_name = _tokens.at(1);

            return result;
        }

        ScanResult parse_type_unknown(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_UNKNOWN;

            result.domain_name = _tokens.at(1);

            return result;
        }


    public:
        static constexpr const char* RESULT_TYPE_INSECURE = "insecure";
        static constexpr const char* RESULT_TYPE_UNRESOLVED = "unresolved";
        static constexpr const char* RESULT_TYPE_SECURE = "secure";
        static constexpr const char* RESULT_TYPE_UNTRUSTWORTHY = "untrustworthy";
        static constexpr const char* RESULT_TYPE_UNKNOWN = "unknown";


        ScanResult parse(const std::string& _line) const
        {
            std::vector<std::string> tokens;
            tokens.reserve(8);
            split_on(_line, ' ', tokens);

            typedef std::function<ScanResult(const ScanResultParser&, const Tokens&)> SubParser;
            const std::map<std::string, SubParser> subparsers_mapping = {
                {RESULT_TYPE_INSECURE, &ScanResultParser::parse_type_insecure},
                {RESULT_TYPE_UNRESOLVED, &ScanResultParser::parse_type_unresolved},
                {RESULT_TYPE_SECURE, &ScanResultParser::parse_type_secure},
                {RESULT_TYPE_UNTRUSTWORTHY, &ScanResultParser::parse_type_untrustworthy},
                {RESULT_TYPE_UNKNOWN, &ScanResultParser::parse_type_unknown},
            };

            if (tokens.size() == 0)
            {
                throw std::runtime_error("zero tokens parsed from scan result");
            }
            try
            {
                auto &subparser = subparsers_mapping.at(tokens.at(0));
                try
                {
                    return subparser(*this, tokens);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error("scan result type " + tokens.at(0) + " parser error");
                }
            }
            catch (const std::out_of_range&)
            {
                throw std::runtime_error("unrecognized/unimplemented scan result type");
            }
        }
    };

    constexpr const char* ScanResultParser::RESULT_TYPE_INSECURE;
    constexpr const char* ScanResultParser::RESULT_TYPE_UNRESOLVED;
    constexpr const char* ScanResultParser::RESULT_TYPE_SECURE;
    constexpr const char* ScanResultParser::RESULT_TYPE_UNTRUSTWORTHY;
    constexpr const char* ScanResultParser::RESULT_TYPE_UNKNOWN;

}

ExternalScannerTool::ExternalScannerTool(const std::string& _external_tool_path)
    : external_tool_path_()
{
    split_on(_external_tool_path, ' ', external_tool_path_);
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
    if (const int ret = pipe(pipes[ParentPipe::READ]) != 0)
    {
        throw std::runtime_error("unable to spawn subprocess (pipe construct failed)");
    }
    if (const int ret = pipe(pipes[ParentPipe::WRITE]) != 0)
    {
        throw std::runtime_error("unable to spawn subprocess (pipe construct failed)");
    }

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

        /* TODO: separate to task serializer */
        long total_tasks = 0;
        const std::string SCAN_MARKER_SECURE = "[secure]\n";
        const std::string SCAN_MARKER_INSECURE = "[insecure]\n";
        for (const auto& nameserver_task : tasks_)
        {
            std::string line_task_secure = "";
            std::string line_task_insecure = nameserver_task.nameserver;
            for (const auto& domain : nameserver_task.nameserver_domains)
            {
                if (domain.has_keyset)
                {
                    if (line_task_secure.size() > 0)
                    {
                        line_task_secure += " ";
                    }
                    line_task_secure += domain.fqdn;
                }
                else
                {
                    line_task_insecure += " " + domain.fqdn;
                }
                total_tasks += 1;
            }
            auto send_line = [&parent_wr_fd](const std::string& _marker, std::string& _line)
            {
                if (_line.size())
                {
                    _line += "\n";
                    {
                        const ssize_t ret = write(parent_wr_fd, _marker.c_str(), _marker.size());
                        if (ret == -1)
                        {
                            throw std::runtime_error("write to pipe failed");
                        }
                        else if (ret != _marker.size())
                        {
                            throw std::runtime_error("write to pipe incomplete?");
                        }
                    }
                    {
                        const ssize_t ret = write(parent_wr_fd, _line.c_str(), _line.size());
                        if (ret == -1)
                        {
                            throw std::runtime_error("write to pipe failed");
                        }
                        else if (ret != _line.size())
                        {
                            throw std::runtime_error("write to pipe incomplete?");
                        }
                    }
                }
            };
            send_line(SCAN_MARKER_SECURE, line_task_secure);
            send_line(SCAN_MARKER_INSECURE, line_task_insecure);
        }
        close(parent_wr_fd);
        std::cout << "total tasks sent: " << total_tasks << std::endl;

        /* TODO: to normal function */
        ScanResultParser scan_result_parser;
        auto buffer_results = [&scan_result_parser](std::string& _raw_buffer, std::vector<ScanResult>& _result_buffer)
        {
            const auto newline_ptr = std::find(_raw_buffer.begin(), _raw_buffer.end(), '\n');
            if (newline_ptr != _raw_buffer.end())
            {
                const auto result_line = std::string(_raw_buffer.begin(), newline_ptr);
                _raw_buffer.erase(_raw_buffer.begin(), newline_ptr + 1);
                try
                {
                    const auto result = scan_result_parser.parse(result_line);
                    const auto filter_out = {
                        ScanResultParser::RESULT_TYPE_UNRESOLVED,
                        ScanResultParser::RESULT_TYPE_UNKNOWN
                    };
                    if (std::find(filter_out.begin(), filter_out.end(), result.cdnskey_status) == filter_out.end())
                    {
                        _result_buffer.emplace_back(result);
                    }
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
