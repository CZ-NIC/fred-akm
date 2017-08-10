#include "src/loader_impl/backend.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


BackendLoader::BackendLoader(const IAkm& _akm_backend) : akm_backend_(_akm_backend)
{
}

void BackendLoader::load_domains(NameserverDomainsCollection& _collection) const
{
    auto merge_collections = [](NameserverDomainsCollection& _dst, const NameserverDomainsCollection& _src)
    {
        for (const auto kv : _src)
        {
            const auto ns = kv.second.nameserver;
            const auto ns_domains = kv.second.nameserver_domains;

            if (_dst.find(ns) == _dst.end())
            {
                _dst[ns] = NameserverDomains(ns, ns_domains);
            }
            else
            {
                auto& ns_domains_dst = _dst[ns].nameserver_domains;
                _dst.reserve(_dst.size() + ns_domains.size());
                std::remove_copy_if(ns_domains.begin(), ns_domains.end(), std::back_inserter(ns_domains_dst),
                        [&ns_domains_dst](const Domain& _check) {
                            return std::find(ns_domains_dst.begin(), ns_domains_dst.end(), _check) != ns_domains_dst.end();
                        }
                );
            }
        }
    };

    merge_collections(_collection, akm_backend_.get_nameservers_with_automatically_managed_domain_candidates());
    merge_collections(_collection, akm_backend_.get_nameservers_with_automatically_managed_domains());

    log()->info("loaded tasks from backend ({} nameserver(s))", _collection.size());
}


} //namespace Akm
} //namespace Fred
