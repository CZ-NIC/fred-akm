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


void DomainWhitelistFilter::apply(NameserverDomainsCollection& _collection) const
{
    NameserverDomainsCollection filtered;
    for (const auto kv : _collection)
    {
        const auto& ns = kv.second.nameserver;
        const auto& ns_domains = kv.second.nameserver_domains;
        for (const auto& domain : ns_domains)
        {
            if (std::find(domain_whitelist_.begin(), domain_whitelist_.end(), domain.fqdn) != domain_whitelist_.end())
            {
                auto& added = filtered[ns];
                added.nameserver = ns;
                added.nameserver_domains.push_back(domain);
                log()->debug("add domain {} to filtered result", domain.fqdn);
            }
        }
    }
    log()->info("tasks filtered ({} nameserver(s))", filtered.size());
    _collection.swap(filtered);
}


} //namespace Akm
} //namespace Fred
