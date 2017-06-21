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

    /* TODO: better change interface of command_load to accept interface of
     * "data loader" and encapsulate load from file to separate impl. to be
     * more testable.
     */
    void command_load(
        const IStorage& _storage,
        const NameserverDomainsCollection& _tasks,
        int _flags)
    {
        if (_flags & LoadFlags::WIPE_QUEUE)
        {
            _storage.wipe_scan_queue();
        }
        if (_flags & LoadFlags::ALLOW_DUPS)
        {
            _storage.append_to_scan_queue(_tasks);
        }
        else
        {
            _storage.append_to_scan_queue_if_not_exists(_tasks);
        }
        if (_flags & LoadFlags::PRUNE)
        {
            _storage.prune_scan_queue();
        }
    }


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


void command_load(const IStorage& _storage, const std::string& _filename, const std::string& _whitelist_filename, int _flags)
{
    auto file = std::ifstream(_filename, std::ifstream::ate | std::ifstream::binary);
    const auto size = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg);

    log()->info("input file size: {} [b]", size);

    NameserverDomainsCollection data;

    std::string line;
    const auto KiB = 1024;
    line.reserve(KiB);

    NameserverDomains ns_domains;
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
                boost::lexical_cast<bool>(tokens[3])
            );

            auto& record = data[current_ns];
            record.nameserver = current_ns;
            record.nameserver_domains.push_back(domain);
        }
        else
        {
            log()->error("not enough tokens skipping (line={})", line);
        }
    }
    log()->info("loaded tasks from input file ({} nameserver(s))", data.size());
    if (_whitelist_filename.length())
    {
        data = Impl::whitelist_filter(data, _whitelist_filename);
    }
    Impl::command_load(_storage, data, _flags);
    log()->info("imported to database");
}


void command_load(const IStorage& _storage, const IAkm& _backend, const std::string& _whitelist_filename, int _flags)
{
    auto data = _backend.get_nameservers_with_automatically_managed_domain_candidates();
    log()->info("loaded tasks from backend ({} nameserver(s))", data.size());
    if (_whitelist_filename.length())
    {
        data = Impl::whitelist_filter(data, _whitelist_filename);
    }
    Impl::command_load(_storage, data, _flags);
    log()->info("imported to database");
}


} //namespace Akm
} //namespace Fred
