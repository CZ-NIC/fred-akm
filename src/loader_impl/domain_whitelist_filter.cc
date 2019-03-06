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
#include "src/loader_impl/domain_whitelist_filter.hh"
#include "src/log.hh"

#include <fstream>
#include <boost/algorithm/string.hpp>

namespace Fred {
namespace Akm {



DomainWhitelistFilter::DomainWhitelistFilter(std::string _filename)
{
    std::ifstream whitelist_file(_filename);
    std::string line;
    const auto KiB = 1024;
    line.reserve(KiB);

    domain_whitelist_.reserve(10000);
    while (std::getline(whitelist_file, line))
    {
        boost::algorithm::trim(line);
        const auto it = std::find(line.begin(), line.end(), ' ');
        if (it == line.end())
        {
            domain_whitelist_.insert(line);
        }
        else
        {
            log()->debug("skipping whitelist file line ({})", line);
        }
    }
}


void DomainWhitelistFilter::apply(DomainScanTaskCollection& _collection) const
{
    DomainScanTaskCollection filtered_collection;
    for (const auto& domain_scan_task : _collection)
    {
        if (std::find(domain_whitelist_.begin(), domain_whitelist_.end(), domain_scan_task.domain.fqdn) != domain_whitelist_.end())
        {
            filtered_collection.insert_or_replace(domain_scan_task);
            log()->debug("add domain {} to filtered result", domain_scan_task.domain.fqdn);
        }
    }
    log()->info("tasks filtered ({} domain(s))", filtered_collection.size());
    _collection.swap(filtered_collection);
}


} //namespace Akm
} //namespace Fred
