#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <boost/lexical_cast.hpp>

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

}


void command_load(const IStorage& _storage, const std::string& _filename, int _flags)
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
    Impl::command_load(_storage, data, _flags);
    log()->info("imported to database");
}


void command_load(const IStorage& _storage, const IAkm& _backend, int _flags)
{
    auto data = _backend.get_nameservers_with_automatically_managed_domain_candidates();
    log()->info("loaded tasks from backend ({} nameserver(s))", data.size());
    Impl::command_load(_storage, data, _flags);
    log()->info("imported to database");
}


} //namespace Akm
} //namespace Fred
