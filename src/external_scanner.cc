#include <map>
#include <array>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

#include "src/utils.hh"
#include "src/external_scanner.hh"
#include "src/log.hh"

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

        ScanResult parse_type_insecure_empty(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_INSECURE_EMPTY;

            result.nameserver = _tokens.at(1);
            result.nameserver_ip = _tokens.at(2);
            result.domain_name = _tokens.at(3);

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

        ScanResult parse_type_secure_empty(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_SECURE_EMPTY;

            result.domain_name = _tokens.at(1);

            return result;
        }

        ScanResult parse_type_unresolved_ip(const Tokens& _tokens) const
        {
            ScanResult result;
            result.cdnskey_status = RESULT_TYPE_UNRESOLVED_IP;

            result.nameserver = _tokens.at(1);

            return result;
        }

    public:
        static constexpr const char* RESULT_TYPE_UNRESOLVED_IP = "unresolved-ip";
        static constexpr const char* RESULT_TYPE_INSECURE = "insecure";
        static constexpr const char* RESULT_TYPE_INSECURE_EMPTY = "insecure-empty";
        static constexpr const char* RESULT_TYPE_UNRESOLVED = "unresolved";
        static constexpr const char* RESULT_TYPE_SECURE = "secure";
        static constexpr const char* RESULT_TYPE_SECURE_EMPTY = "secure-empty";
        static constexpr const char* RESULT_TYPE_UNTRUSTWORTHY = "untrustworthy";
        static constexpr const char* RESULT_TYPE_UNKNOWN = "unknown";


        ScanResult parse(const std::string& _line) const
        {
            std::vector<std::string> tokens;
            tokens.reserve(8);
            split_on(_line, ' ', tokens);

            typedef std::function<ScanResult(const ScanResultParser&, const Tokens&)> SubParser;
            const std::map<std::string, SubParser> subparsers_mapping = {
                {RESULT_TYPE_UNRESOLVED_IP, &ScanResultParser::parse_type_unresolved_ip},
                {RESULT_TYPE_INSECURE, &ScanResultParser::parse_type_insecure},
                {RESULT_TYPE_INSECURE_EMPTY, &ScanResultParser::parse_type_insecure_empty},
                {RESULT_TYPE_UNRESOLVED, &ScanResultParser::parse_type_unresolved},
                {RESULT_TYPE_SECURE, &ScanResultParser::parse_type_secure},
                {RESULT_TYPE_SECURE_EMPTY, &ScanResultParser::parse_type_secure_empty},
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
                    throw std::runtime_error("scan result type '" + tokens.at(0) + "' parser error");
                }
            }
            catch (const std::out_of_range&)
            {
                throw std::runtime_error("unrecognized/unimplemented scan result type");
            }
        }
    };

    constexpr const char* ScanResultParser::RESULT_TYPE_UNRESOLVED_IP;
    constexpr const char* ScanResultParser::RESULT_TYPE_INSECURE;
    constexpr const char* ScanResultParser::RESULT_TYPE_INSECURE_EMPTY;
    constexpr const char* ScanResultParser::RESULT_TYPE_UNRESOLVED;
    constexpr const char* ScanResultParser::RESULT_TYPE_SECURE;
    constexpr const char* ScanResultParser::RESULT_TYPE_SECURE_EMPTY;
    constexpr const char* ScanResultParser::RESULT_TYPE_UNTRUSTWORTHY;
    constexpr const char* ScanResultParser::RESULT_TYPE_UNKNOWN;


    class ScanTaskSerializer
    {
    private:
        unsigned long long insecure_domains_counter;
        unsigned long long secure_domains_counter;

    public:
        ScanTaskSerializer() : insecure_domains_counter(0), secure_domains_counter(0) { }

        unsigned long long get_insecure_domains_counter() const
        {
            return insecure_domains_counter;
        }

        unsigned long long get_secure_domains_counter() const
        {
            return secure_domains_counter;
        }

        template<class Writter>
        void serialize_insecure(const NameserverDomainsCollection& _tasks, Writter& _writter)
        {
            bool insecure_marker_written = false;
            for (const auto kv : _tasks)
            {
                const auto& task = kv.second;
                std::string line;
                for (const auto& domain : task.nameserver_domains)
                {
                    if (domain.has_keyset == false)
                    {
                        if (!insecure_marker_written)
                        {
                            _writter("[insecure]\n");
                            insecure_marker_written = true;
                        }
                        line.append(" " + domain.fqdn);
                        insecure_domains_counter += 1;
                    }
                }
                if (insecure_marker_written && !line.empty())
                {
                    _writter(task.nameserver + line + "\n");
                }
            }
        }

        template<class Writter>
        void serialize_secure(const NameserverDomainsCollection& _tasks, Writter& _writter)
        {
            std::unordered_set<std::string> written_domains;
            bool secure_marker_written = false;
            const auto DELIMITER = " ";
            for (const auto kv : _tasks)
            {
                const auto& task = kv.second;
                for (const auto& domain : task.nameserver_domains)
                {
                    if (domain.has_keyset == true && written_domains.count(domain.fqdn) == 0)
                    {
                        if (!secure_marker_written)
                        {
                            _writter("[secure]\n");
                            _writter(domain.fqdn);
                            secure_marker_written = true;
                        }
                        else
                        {
                            _writter(DELIMITER + domain.fqdn);
                        }
                        written_domains.insert(domain.fqdn);
                        secure_domains_counter += 1;
                    }
                }
            }
            if (secure_marker_written)
            {
                _writter("\n");
            }
        }
    };


    unsigned long long compute_best_runtime_for_input(
        const NameserverDomainsCollection& _tasks,
        unsigned long long _queries_per_second
    )
    {
        auto domains_count = 0;
        for (const auto kv : _tasks)
        {
            domains_count += kv.second.nameserver_domains.size();
        }

        const auto queries_estimate = _tasks.size() + 2 * domains_count;
        const auto runtime = queries_estimate / _queries_per_second;
        log()->debug("runtime: {} [s] [nameservers:{} domains:{} queries-estimate:{}]",
            runtime, _tasks.size(), domains_count, queries_estimate
        );
        return runtime;
    }


}



ExternalScannerTool::ExternalScannerTool(const std::string& _external_tool_path)
    :  external_tool_path_()
{
    split_on(_external_tool_path, ' ', external_tool_path_);
    if (external_tool_path_.size() < 2)
    {
        throw std::runtime_error("no scanner tool path supplied");
    }
}


void ExternalScannerTool::scan(const NameserverDomainsCollection& _tasks, OnResultsCallback _callback) const
{
    // auto runtime = compute_best_runtime_for_input(_tasks, queries_per_second_);
    // auto runtime_arg = boost::lexical_cast<std::string>(runtime);

    std::vector<const char*> subprocess_argv;
    for (const auto& path_part : external_tool_path_)
    {
        /* path_part.c_str() is owned by external_tool_path_ vector */
        subprocess_argv.push_back(path_part.c_str());
    }
    // subprocess_argv.push_back(runtime_arg.c_str());
    subprocess_argv.push_back(nullptr);


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

    const pid_t parent_pid = getpid();
    const pid_t child_pid = fork();
    if (child_pid == -1)
    {
        throw std::runtime_error("running subprocess failed (fork() failed)");
    }
    else if (child_pid == 0)
    {
        /* child */
        dup2(child_rd_fd, STDIN_FILENO);
        dup2(child_wr_fd, STDOUT_FILENO);

        close(child_rd_fd);
        close(child_wr_fd);
        close(parent_rd_fd);
        close(parent_wr_fd);

        ::execv(subprocess_argv[0], const_cast<char **>(&subprocess_argv[0]));
        throw std::runtime_error("external scanner tool subprocess call failed");
    }
    else
    {
        log()->debug("parent_pid={} child_pid={}", parent_pid, child_pid);

        close(child_rd_fd);
        close(child_wr_fd);

        auto write_to_scanner = [&parent_wr_fd](const std::string& _str)
        {
            if (_str.size() > 0)
            {
                const ssize_t ret = write(parent_wr_fd, _str.c_str(), _str.size());
                if (ret == -1)
                {
                    throw std::runtime_error("write to pipe failed");
                }
                else if (ret != _str.size())
                {
                    throw std::runtime_error("write to pipe incomplete?");
                }
            }
        };

        auto write_to_stdout = [](const std::string& _str)
        {
            if (_str.size() > 0)
            {
                std::cout << _str;
            }
        };

        ScanTaskSerializer serializer;
        serializer.serialize_insecure(_tasks, write_to_scanner);
        serializer.serialize_secure(_tasks, write_to_scanner);

        close(parent_wr_fd);
        log()->info("total domains for scan sent: {} (insecure={} secure={})",
            serializer.get_insecure_domains_counter() + serializer.get_secure_domains_counter(),
            serializer.get_insecure_domains_counter(),
            serializer.get_secure_domains_counter()
        );

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
                    _result_buffer.emplace_back(scan_result_parser.parse(result_line));
                }
                catch (const std::exception& ex)
                {
                    log()->error("buffer[!]: {}", result_line);
                    log()->error("buffer[!]: {}", ex.what());
                }
                return true;
            }
            return false;
        };

        long total_results = 0;

        const auto RESULT_BUFFER_ITEM_SIZE = 1024;
        std::vector<ScanResult> result_buffer;
        result_buffer.reserve(RESULT_BUFFER_ITEM_SIZE);

        const auto READ_CHUNK_SIZE = 512;
        std::array<char, READ_CHUNK_SIZE> chunk;
        std::string raw_buffer;

        log()->debug("result buffer size: {} item(s)", RESULT_BUFFER_ITEM_SIZE);
        log()->debug("read chunk buffer size: {} [b]", READ_CHUNK_SIZE);
        log()->info("waiting for results...");
        int read_count = 0;
        int child_status;
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
                        log()->debug("buffer[-]: buffer full - saving results (saved={} total={})",
                            result_buffer.size(), total_results
                        );
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
            total_results += result_buffer.size();
        }
        const pid_t exited_child = waitpid(child_pid, &child_status, 0);
        log()->debug("child exit-status:{}", WEXITSTATUS(child_status));
        if (exited_child == child_pid)
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

        log()->info("total proccessed results: {}", total_results);
    }
}


} //namespace Akm
} //namespace Fred
