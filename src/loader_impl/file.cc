/*
 * Copyright (C) 2017  CZ.NIC, z. s. p. o.
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
#include "src/loader_impl/file.hh"
#include "src/log.hh"
#include "src/utils.hh"

#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace Fred {
namespace Akm {

namespace {

    void load_tasks_from_file(std::string _filename, DomainScanTaskCollection& _scan_tasks)
    {
        std::ifstream file(_filename, std::ifstream::ate | std::ifstream::binary);
        const auto size = file.tellg();
        file.clear();
        file.seekg(0, std::ios::beg);

        log()->info("input file size: {} [b]", size);

        std::string line;
        const auto KiB = 1024;
        line.reserve(KiB);

        while (std::getline(file, line))
        {
            std::vector<std::string> tokens;
            tokens.reserve(4);
            split_on(line, ' ', tokens);

            if (tokens.size() == 4)
            {
                const auto& current_ns = tokens[0];
                const auto domain = Domain(
                    boost::lexical_cast<unsigned long long>(tokens[1]),
                    tokens[2],
                    from_string<ScanType>(tokens[3])
                );

                _scan_tasks.insert_or_update(domain, current_ns);
            }
            else
            {
                log()->error("not enough tokens skipping (line={})", line);
            }
        }
    }

    void filter_scan_type(const DomainScanTaskCollection& _in, DomainScanTaskCollection& _out, ScanType _scan_type)
    {
        for (const auto& task : _in)
        {
            if (task.domain.scan_type == _scan_type)
            {
                _out.insert_or_update(task);
            }
        }
    }

}

FileLoader::FileLoader(std::string _filename)
{
    load_tasks_from_file(std::move(_filename), scan_tasks_);
}


void FileLoader::load_insecure_tasks(DomainScanTaskCollection& _scan_tasks) const
{
    DomainScanTaskCollection tasks;
    filter_scan_type(scan_tasks_, tasks, ScanType::insecure);
    _scan_tasks.merge(tasks);
    log()->info("loaded insecure tasks from backend ({} domain(s))", tasks.size());
}


void FileLoader::load_secure_auto_tasks(DomainScanTaskCollection& _scan_tasks) const
{
    DomainScanTaskCollection tasks;
    filter_scan_type(scan_tasks_, tasks, ScanType::secure_auto);
    _scan_tasks.merge(tasks);
    log()->info("loaded secure-auto tasks from backend ({} domain(s))", tasks.size());
}


void FileLoader::load_secure_noauto_tasks(DomainScanTaskCollection& _scan_tasks) const
{
    DomainScanTaskCollection tasks;
    filter_scan_type(scan_tasks_, tasks, ScanType::secure_noauto);
    _scan_tasks.merge(tasks);
    log()->info("loaded secure-noauto tasks from backend ({} domain(s))", tasks.size());
}


} //namespace Akm
} //namespace Fred
