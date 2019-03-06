/*
 * Copyright (C) 2017-2018  CZ.NIC, z. s. p. o.
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
#include <iostream>
#include <unordered_set>
#include <boost/lexical_cast.hpp>

#include "src/utils.hh"
#include "src/subprocess.hh"
#include "src/cdnskey_scanner_impl/cdnskey_scanner_impl.hh"
#include "src/cdnskey_scanner_impl/input_serializer.hh"
#include "src/cdnskey_scanner_impl/output_parser.hh"
#include "src/log.hh"


namespace Fred {
namespace Akm {

class ScanResultBuffer
{
private:
    unsigned long long total_results_;
    std::vector<ScanResult> buffer_;
    ExternalCdnskeyScannerImpl::OnResultsCallback on_full_buffer_callback_;

public:
    ScanResultBuffer(
        ExternalCdnskeyScannerImpl::OnResultsCallback _on_full_buffer_callback,
        const unsigned long long _max_buffer_items = 1024
    )
        : total_results_(0), on_full_buffer_callback_(_on_full_buffer_callback)
    {
        buffer_.reserve(_max_buffer_items);
    }

    unsigned long long get_max_buffer_items() const
    {
        return buffer_.capacity();
    }

    unsigned long long get_total_results() const
    {
        return total_results_;
    }

    void add(const ScanResult& _scan_result)
    {
        if (buffer_.size() == buffer_.capacity())
        {
            this->process_results();
        }
        buffer_.emplace_back(_scan_result);
        total_results_ += 1;
    }

    void add(const std::vector<ScanResult>& _scan_results)
    {
        for (const auto& item : _scan_results)
        {
            this->add(item);
        }
    }

    void process_results()
    {
        log()->debug("buffer[-]: buffer full - saving results (saved={} total={})", buffer_.size(), total_results_);
        on_full_buffer_callback_(buffer_);
        buffer_.clear();
    }
};


ExternalCdnskeyScannerImpl::ExternalCdnskeyScannerImpl(const std::string& _scanner_path)
    :  scanner_path_()
{
    split_on(_scanner_path, ' ', scanner_path_);
    if (scanner_path_.size() < 2)
    {
        throw std::runtime_error("no scanner tool path supplied");
    }
}


void ExternalCdnskeyScannerImpl::scan(const DomainScanTaskCollection& _tasks, OnResultsCallback _callback) const
{
    std::vector<const char*> subprocess_argv;
    for (const auto& path_part : scanner_path_)
    {
        /* path_part.c_str() is owned by scanner_path_ vector */
        subprocess_argv.push_back(path_part.c_str());
    }
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
    serializer.serialize(_tasks, scanner_writter);

    scanner_subprocess.write_eof();

    log()->info("total domains for scan sent: {} (insecure={} secure={})",
        serializer.get_insecure_domains_counter() + serializer.get_secure_domains_counter(),
        serializer.get_insecure_domains_counter(),
        serializer.get_secure_domains_counter()
    );

    ScanResultParser result_parser;
    ScanResultBuffer result_buffer(_callback);
    Subprocess::ReadBuffer chunk;
    std::string raw_buffer;

    log()->debug("result buffer size: {} item(s)", result_buffer.get_max_buffer_items());
    log()->info("waiting for results...");
    int read_count = 0;
    while ((read_count = scanner_subprocess.read(chunk)) > 0)
    {
        if (read_count >= 0)
        {
            chunk[read_count] = 0;
            raw_buffer += std::string(chunk.data());
            chunk = {{}};
            try
            {
                result_buffer.add(result_parser.parse_multi(raw_buffer));
            }
            catch (const std::exception& ex)
            {
                log()->error("buffer[!]: {}", ex.what());
            }
        }
        else
        {
            throw std::runtime_error("IOError");
        }
    }
    result_buffer.process_results();

    const int child_exit_status = scanner_subprocess.wait();
    log()->debug("child exit-status:{}", child_exit_status);
    log()->info("total proccessed results: {}", result_buffer.get_total_results());
}


} //namespace Akm
} //namespace Fred
