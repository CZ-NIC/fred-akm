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
#ifndef INPUT_SERIALIZER_HH_F4E5BB7519898756BDB5624932F3E8C6//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define INPUT_SERIALIZER_HH_F4E5BB7519898756BDB5624932F3E8C6

#include "src/scan_task.hh"

#include <string>
#include <unordered_set>

namespace Fred {
namespace Akm {


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
    void serialize_insecure(const NameserverToDomainScanTaskAdapter& _tasks, Writter& _writter)
    {
        std::unordered_set<std::string> written_domains;
        bool insecure_marker_written = false;
        for (const auto& kv : _tasks)
        {
            const auto& nameserver = kv.first;
            std::string line;
            for (const auto task : _tasks.find_all(nameserver))
            {
                const auto& domain = task->domain;
                if (domain.scan_type == ScanType::insecure)
                {
                    if (!insecure_marker_written)
                    {
                        _writter("[insecure]\n");
                        insecure_marker_written = true;
                    }
                    line.append(" " + domain.fqdn);
                    if (written_domains.count(domain.fqdn) == 0)
                    {
                        insecure_domains_counter += 1;
                        written_domains.insert(domain.fqdn);
                    }
                }
            }
            if (insecure_marker_written && !line.empty())
            {
                _writter(nameserver + line + "\n");
            }
        }
    }

    template<class Writter>
    void serialize_secure(const NameserverToDomainScanTaskAdapter& _tasks, Writter& _writter)
    {
        std::unordered_set<std::string> written_domains;
        bool secure_marker_written = false;
        const auto DELIMITER = " ";
        for (const auto& kv : _tasks)
        {
            const auto& nameserver = kv.first;
            for (const auto task : _tasks.find_all(nameserver))
            {
                const auto& domain = task->domain;
                if ((domain.scan_type == ScanType::secure_auto
                    || domain.scan_type == ScanType::secure_noauto)
                    && written_domains.count(domain.fqdn) == 0)
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

    template<class Writter>
    void serialize(const NameserverToDomainScanTaskAdapter& _tasks, Writter& _writter)
    {
        serialize_insecure(_tasks, _writter);
        serialize_secure(_tasks, _writter);
    }

    template<class Writter>
    void serialize(const DomainScanTaskCollection& _tasks, Writter& _writter)
    {
        NameserverToDomainScanTaskAdapter tasks_by_nameserver(_tasks);
        serialize(tasks_by_nameserver, _writter);
    }
};


} // namespace Akm
} // namespace Fred


#endif//INPUT_SERIALIZER_HH_F4E5BB7519898756BDB5624932F3E8C6
