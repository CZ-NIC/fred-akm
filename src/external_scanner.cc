#include <iostream>
#include <unordered_set>
#include <boost/lexical_cast.hpp>

#include "src/utils.hh"
#include "src/subprocess.hh"
#include "src/external_scanner.hh"
#include "src/cdnskey_scanner_impl/input_serializer.hh"
#include "src/cdnskey_scanner_impl/output_parser.hh"
#include "src/log.hh"


namespace Fred {
namespace Akm {

namespace {

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

    Subprocess scanner_subprocess(subprocess_argv);
    log()->debug("parent_pid={} child_pid={}", scanner_subprocess.parent_pid(), scanner_subprocess.child_pid());

    auto scanner_writter = [&scanner_subprocess](const std::string& _str)
    {
        scanner_subprocess.write(_str);
    };

    auto stdout_writter = [](const std::string& _str)
    {
        if (_str.size() > 0)
        {
            std::cout << _str;
        }
    };

    ScanTaskSerializer serializer;
    serializer.serialize_insecure(_tasks, scanner_writter);
    serializer.serialize_secure(_tasks, scanner_writter);

    scanner_subprocess.write_eof();

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

    Subprocess::ReadBuffer chunk;
    std::string raw_buffer;

    log()->debug("result buffer size: {} item(s)", RESULT_BUFFER_ITEM_SIZE);
    log()->info("waiting for results...");
    int read_count = 0;
    int child_status;
    while ((read_count = scanner_subprocess.read(chunk)) > 0)
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

    const int child_exit_status = scanner_subprocess.wait();
    log()->debug("child exit-status:{}", child_exit_status);
    log()->info("total proccessed results: {}", total_results);
}


} //namespace Akm
} //namespace Fred
