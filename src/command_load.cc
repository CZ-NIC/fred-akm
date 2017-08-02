#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "src/utils.hh"
#include "src/log.hh"
#include "src/command_load.hh"

namespace Fred {
namespace Akm {

namespace Impl {

    NameserverDomainsCollection whitelist_filter(const NameserverDomainsCollection& _tasks, const std::string& _whitelist_filename)
    {
        std::ifstream whitelist_file(_whitelist_filename);
        std::string line;
        const auto KiB = 1024;
        line.reserve(KiB);

        std::vector<std::string> whitelist_domains;
        whitelist_domains.reserve(10000);
        while (std::getline(whitelist_file, line))
        {
            boost::algorithm::trim(line);
            const auto it = std::find(line.begin(), line.end(), ' ');
            if (it == line.end())
            {
                whitelist_domains.push_back(line);
            }
            else
            {
                log()->debug("skipping whitelist file line ({})", line);
            }
        }

        NameserverDomainsCollection filtered;
        for (const auto kv : _tasks)
        {
            const auto& ns = kv.second.nameserver;
            const auto& ns_domains = kv.second.nameserver_domains;
            for (const auto& domain : ns_domains)
            {
                if (std::find(whitelist_domains.begin(), whitelist_domains.end(), domain.fqdn) != whitelist_domains.end())
                {
                    auto& added = filtered[ns];
                    added.nameserver = ns;
                    added.nameserver_domains.push_back(domain);
                    log()->debug("add domain {} to filtered result", domain.fqdn);
                }
            }
        }
        log()->info("tasks filtered ({} nameserver(s))", filtered.size());
        return filtered;
    }

}


void command_load(
    const IStorage& _storage,
    const ILoader& _loader,
    const std::string& _whitelist_filename,
    int _flags)
{
    NameserverDomainsCollection data;
    _loader.load_domains(data);

    if (_whitelist_filename.length())
    {
        data = Impl::whitelist_filter(data, _whitelist_filename);
        log()->info("forcing scan queue wipe on filtered input");
        _flags |= LoadFlags::WIPE_QUEUE;
    }

    if (_flags & LoadFlags::WIPE_QUEUE)
    {
        _storage.wipe_scan_queue();
    }
    if (_flags & LoadFlags::ALLOW_DUPS)
    {
        _storage.append_to_scan_queue(data);
    }
    else
    {
        _storage.append_to_scan_queue_if_not_exists(data);
    }
    if (_flags & LoadFlags::PRUNE)
    {
        _storage.prune_scan_queue();
    }

    log()->info("imported to database");
}


} //namespace Akm
} //namespace Fred
