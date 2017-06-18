#include "src/corba/akm.hh"
#include "src/corba/generated/AutomaticKeysetManagement.hh"

namespace Fred {
namespace Akm {
namespace Corba {


Akm::Akm(const Nameservice& _ns, const std::string& _ns_path_akm)
    : ns_(_ns), ns_path_akm_(_ns_path_akm)
{
}


NameserverDomainsCollection Akm::get_nameservers_with_automatically_managed_domain_candidates() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_automatically_managed_domain_candidates();

        NameserverDomainsCollection all_nameserver_domains;

        for (unsigned long long i = 0; i < c_all_nameserver_domains->length(); ++i)
        {
            NameserverDomains one_ns;
            one_ns.nameserver = c_all_nameserver_domains[i].nameserver;
            one_ns.nameserver_domains.reserve(c_all_nameserver_domains[i].nameserver_domains.length());
            for (unsigned long long j = 0; j < c_all_nameserver_domains[i].nameserver_domains.length(); ++j)
            {
                const auto& c_domain = c_all_nameserver_domains[i].nameserver_domains[j];
                one_ns.nameserver_domains.emplace_back(Fred::Akm::Domain(c_domain.id, std::string(c_domain.fqdn), false));
            }
            all_nameserver_domains[one_ns.nameserver] = one_ns;
        }

        return all_nameserver_domains;
    }
    catch (const CORBA::SystemException &e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception &e)
    {
        throw std::runtime_error(e._name());
    }
}


} // namespace Corba
} // namespace Akm
} // namespace Fred
