#include "src/loader_impl/backend.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


BackendLoader::BackendLoader(const IAkm& _akm_backend) : akm_backend_(_akm_backend)
{
}

void BackendLoader::load_domains(NameserverDomainsCollection& _collection) const
{
    auto data = akm_backend_.get_nameservers_with_automatically_managed_domain_candidates();
    for (const auto kv : akm_backend_.get_nameservers_with_automatically_managed_domains())
    {
        const auto ns = kv.second.nameserver;
        const auto ns_domains = kv.second.nameserver_domains;

        if (data.find(ns) == data.end())
        {
            data[ns] = NameserverDomains(ns, ns_domains);
        }
        else
        {
            auto& dest = _collection[ns].nameserver_domains;
            dest.reserve(dest.size() + ns_domains.size());
            std::copy(ns_domains.begin(), ns_domains.end(), std::back_inserter(dest));
        }
    }
    log()->info("loaded tasks from backend ({} nameserver(s))", _collection.size());
}


} //namespace Akm
} //namespace Fred
