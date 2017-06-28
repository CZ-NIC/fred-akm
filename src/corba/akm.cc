#include "src/corba/akm.hh"
#include "src/corba/generated/AutomaticKeysetManagement.hh"

namespace Fred {
namespace Akm {
namespace Corba {


Akm::Akm(const Nameservice& _ns, const std::string& _ns_path_akm)
    : ns_(_ns), ns_path_akm_(_ns_path_akm)
{
}

namespace {

NameserverDomainsCollection unwrap_NameserverDomains(
    Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var c_all_nameserver_domains,
    bool _has_keyset
)
{
    NameserverDomainsCollection all_nameserver_domains;

    for (unsigned long long i = 0; i < c_all_nameserver_domains->length(); ++i)
    {
        NameserverDomains one_ns;
        one_ns.nameserver = c_all_nameserver_domains[i].nameserver;
        one_ns.nameserver_domains.reserve(c_all_nameserver_domains[i].nameserver_domains.length());
        for (unsigned long long j = 0; j < c_all_nameserver_domains[i].nameserver_domains.length(); ++j)
        {
            const auto& c_domain = c_all_nameserver_domains[i].nameserver_domains[j];
            one_ns.nameserver_domains.emplace_back(Fred::Akm::Domain(c_domain.id, std::string(c_domain.fqdn), _has_keyset));
        }
        all_nameserver_domains[one_ns.nameserver] = one_ns;
    }

    return all_nameserver_domains;
}

TechContacts unwrap_TechContactSeq(Registry::AutomaticKeysetManagement::TechContactSeq_var _tech_contacts)
{
        TechContacts tech_contacts;
        tech_contacts.reserve(_tech_contacts->length());

        for (unsigned long long i = 0; i < _tech_contacts->length(); ++i)
        {
            const auto& c_tech_contact = _tech_contacts[i];
            tech_contacts.emplace_back(c_tech_contact);
        }

        return tech_contacts;
}

Registry::AutomaticKeysetManagement::Nsset wrap_Nsset(Nsset _nsset)
{
    Registry::AutomaticKeysetManagement::Nsset nsset;

    CORBA::ULong i = 0;
    nsset.nameservers.length(_nsset.nameservers.size());
    for (const auto& nameserver : _nsset.nameservers)
    {
        nsset.nameservers[i] = nameserver.c_str();
        ++i;
    }
    return nsset;
}

Registry::AutomaticKeysetManagement::Keyset wrap_Keyset(Keyset _keyset)
{
    Registry::AutomaticKeysetManagement::Keyset keyset;

    CORBA::ULong i = 0;
    keyset.dns_keys.length(_keyset.dnskeys.size());
    for (const auto& dnskey : _keyset.dnskeys)
    {
        keyset.dns_keys[i].flags = dnskey.flags;
        keyset.dns_keys[i].protocol = dnskey.proto;
        keyset.dns_keys[i].alg = dnskey.alg;
        keyset.dns_keys[i].public_key = dnskey.public_key.c_str();
        ++i;
    }
    return keyset;
}

} // Fred::Akm::{anonymous}

NameserverDomainsCollection Akm::get_nameservers_with_automatically_managed_domain_candidates() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_automatically_managed_domain_candidates();

        return unwrap_NameserverDomains(c_all_nameserver_domains, false);
    }
    catch (const CORBA::SystemException& e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception& e)
    {
        throw std::runtime_error(e._name());
    }
}

NameserverDomainsCollection Akm::get_nameservers_with_automatically_managed_domains() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_automatically_managed_domains();

        return unwrap_NameserverDomains(c_all_nameserver_domains, true);
    }
    catch (const CORBA::SystemException& e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception& e)
    {
        throw std::runtime_error(e._name());
    }
}

TechContacts Akm::get_nsset_notification_emails_by_domain_id(unsigned long long _domain_id) const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm =
                AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::TechContactSeq_var c_tech_contacts =
                akm->get_nsset_notification_emails_by_domain_id(_domain_id);

        return unwrap_TechContactSeq(c_tech_contacts);
    }
    catch (const CORBA::SystemException& e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception& e)
    {
        throw std::runtime_error(e._name());
    }
}

void Akm::update_domain_automatic_keyset(unsigned long long _domain_id, Nsset _current_nsset, Keyset _new_keyset) const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm =
                AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));

        AkmModule::Nsset current_nsset = wrap_Nsset(_current_nsset);
        AkmModule::Keyset new_keyset = wrap_Keyset(_new_keyset);

        akm->update_domain_automatic_keyset(_domain_id, current_nsset, new_keyset);

    }
    catch (const CORBA::SystemException& e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception& e)
    {
        throw std::runtime_error(e._name());
    }
}

} // namespace Corba
} // namespace Akm
} // namespace Fred
