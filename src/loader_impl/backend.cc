#include "src/loader_impl/backend.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


BackendLoader::BackendLoader(const IAkm& _akm_backend) : akm_backend_(_akm_backend)
{
}


void BackendLoader::load_domains(DomainScanTaskCollection& _collection) const
{
    _collection.merge(akm_backend_.get_nameservers_with_insecure_automatically_managed_domain_candidates());
    _collection.merge(akm_backend_.get_nameservers_with_automatically_managed_domains());
    _collection.merge(akm_backend_.get_nameservers_with_secure_automatically_managed_domain_candidates());

    log()->info("loaded tasks from backend ({} domain(s))", _collection.size());
}


} //namespace Akm
} //namespace Fred
