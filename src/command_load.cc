#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "src/utils.hh"
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
        const std::vector<NameserverDomains>& _tasks,
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
    const auto output_prefix = "[command::load] ";

    auto file = std::ifstream(_filename, std::ifstream::ate | std::ifstream::binary);
    const auto size = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg);

    std::cout << output_prefix << "input file size: " << size << " [b]" << std::endl;

    std::vector<NameserverDomains> data;
    // data.reserve(?);

    std::string line;
    const auto KiB = 1024;
    line.reserve(KiB);

    NameserverDomains ns_domains;
    // ns_domains.nameserver_domains.reserve(?);

    const char DELIMITER = ',';
    while (std::getline(file, line))
    {
        std::cout << line << std::endl;
        std::vector<std::string> parsed;
        auto b = line.begin();
        auto e = line.end();
        auto n = std::find(b, e, DELIMITER);
        while (n != e)
        {
            parsed.emplace_back(std::string(b, n));
            b = n + 1;
            n = std::find(b, e, DELIMITER);
        }
        parsed.emplace_back(std::string(b, n));

        if (parsed.size() == 3)
        {
            const auto& current_ns = parsed[0];

            if (ns_domains.nameserver != current_ns)
            {
                if (!ns_domains.nameserver_domains.empty())
                {
                    data.push_back(ns_domains);
                    ns_domains.nameserver_domains.clear();
                    if (data.size() % 1000 == 0)
                    {
                        std::cout << output_prefix << "checkpoint: " << data.size() << std::endl;
                    }
                }
                ns_domains.nameserver = current_ns;
            }
            ns_domains.nameserver_domains.emplace_back(Domain(boost::lexical_cast<unsigned long long>(parsed[1]), parsed[2]));
        }
        else
        {
            std::cout << output_prefix << "skipping.." << std::endl;
        }
    }
    std::cout << output_prefix << "parsed input file (" << data.size() << " nameserver(s))" << std::endl;

    Impl::command_load(_storage, data, _flags);
    std::cout << output_prefix << "imported to database" << std::endl;
}


void command_load(const IStorage& _storage, const IAkm& _backend, int _flags)
{
    const auto output_prefix = "[command::load] ";
    auto data = _backend.get_nameservers_with_automatically_managed_domain_candidates();
    std::cout << output_prefix << "loaded data from backend (" << data.size() << " nameserver(s))" << std::endl;
    Impl::command_load(_storage, data, _flags);
    std::cout << output_prefix << "imported to database" << std::endl;
}


} //namespace Akm
} //namespace Fred
