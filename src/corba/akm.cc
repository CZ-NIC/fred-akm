#include "src/corba/akm.hh"
#include "src/corba/generated/AutomaticKeysetManagement.hh"

namespace Fred {
namespace Akm {
namespace Corba {


Akm::Akm(const Nameservice& _ns, const std::string& _ns_path_akm)
    : ns_(_ns), ns_path_akm_(_ns_path_akm)
{
}


std::vector<NameserverDomains> Akm::get_nameservers_with_automatically_managed_domain_candidates() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_nameserver_domains = akm->get_nameservers_with_automatically_managed_domain_candidates();

        std::vector<NameserverDomains> nameserver_domains;
        nameserver_domains.reserve(c_nameserver_domains->length());

        for (unsigned long long i = 0; i < c_nameserver_domains->length(); ++i)
        {
            NameserverDomains ns_domains;
            ns_domains.nameserver = c_nameserver_domains[i].nameserver;
            ns_domains.nameserver_domains.reserve(c_nameserver_domains[i].nameserver_domains.length());
            for (unsigned long long j = 0; j < c_nameserver_domains[i].nameserver_domains.length(); ++j)
            {
                const auto& c_domain = c_nameserver_domains[i].nameserver_domains[j];
                ns_domains.nameserver_domains.emplace_back(Fred::Akm::Domain(c_domain.id, std::string(c_domain.fqdn), false));
            }
            nameserver_domains.emplace_back(ns_domains);
        }

        return nameserver_domains;
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
