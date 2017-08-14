#include "src/corba/akm.hh"
#include "src/corba/generated/AutomaticKeysetManagement.hh"
#include "src/scan_task.hh"

#include <algorithm>

namespace Fred {
namespace Akm {
namespace Corba {


struct ObjectNotFound : AkmException
{
    const char* what() const throw () {
        return "object not found";
    }
};


struct NssetInvalid : AkmException
{
    const char* what() const throw () {
        return "nsset invalid ";
    }
};


struct KeysetInvalid : AkmException
{
    const char* what() const throw () {
        return "keyset invalid ";
    }
};


struct NssetDiffers : AkmException
{
    const char* what() const throw () {
        return "nsset differs ";
    }
};


struct DomainHasOtherKeyset : AkmException
{
    const char* what() const throw () {
        return "domain hasother keyset ";
    }
};


struct DomainStatePolicyError : AkmException
{
    const char* what() const throw () {
        return "domain state policy error ";
    }
};


struct KeysetStatePolicyError : AkmException
{
    const char* what() const throw () {
        return "keyset state policy error ";
    }
};


struct SystemRegistratorNotFound : AkmException
{
    const char* what() const throw () {
        return "system registrator not found ";
    }
};


struct ConfigurationError : AkmException
{
    const char* what() const throw () {
        return "configuration error ";
    }
};


struct InternalServerError : AkmException
{
    const char* what() const throw () {
        return "internal server error ";
    }
};


Akm::Akm(const Nameservice& _ns, const std::string& _ns_path_akm)
    : ns_(_ns), ns_path_akm_(_ns_path_akm)
{
}

namespace {

DomainScanTaskCollection unwrap_NameserverDomains(
    Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var c_all_nameserver_domains,
    Fred::Akm::ScanType _scan_type
)
{
    DomainScanTaskCollection collection;

    for (unsigned long long i = 0; i < c_all_nameserver_domains->length(); ++i)
    {
        const std::string nameserver(std::string(c_all_nameserver_domains[i].nameserver));

        for (unsigned long long j = 0; j < c_all_nameserver_domains[i].nameserver_domains.length(); ++j)
        {
            const auto& c_domain = c_all_nameserver_domains[i].nameserver_domains[j];
            collection.insert_or_update(Fred::Akm::Domain(c_domain.id, std::string(c_domain.fqdn), _scan_type), nameserver);
        }
    }

    return collection;
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


DomainScanTaskCollection Akm::get_nameservers_with_automatically_managed_domain_candidates() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_automatically_managed_domain_candidates();

        return unwrap_NameserverDomains(c_all_nameserver_domains, ScanType::insecure);
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


DomainScanTaskCollection Akm::get_nameservers_with_automatically_managed_domains() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_automatically_managed_domains();

        return unwrap_NameserverDomains(c_all_nameserver_domains, ScanType::secure_auto);
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
    catch (const Registry::AutomaticKeysetManagement::OBJECT_NOT_FOUND&)
    {
        throw ObjectNotFound();
    }
    catch (const Registry::AutomaticKeysetManagement::NSSET_INVALID&)
    {
        throw NssetInvalid();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_INVALID&)
    {
        throw KeysetInvalid();
    }
    catch (const Registry::AutomaticKeysetManagement::NSSET_DIFFERS&)
    {
        throw NssetDiffers();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_HAS_OTHER_KEYSET&)
    {
        throw DomainHasOtherKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_STATE_POLICY_ERROR&)
    {
        throw DomainStatePolicyError();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_STATE_POLICY_ERROR&)
    {
        throw KeysetStatePolicyError();
    }
    catch (const Registry::AutomaticKeysetManagement::SYSTEM_REGISTRATOR_NOT_FOUND&)
    {
        throw SystemRegistratorNotFound();
    }
    catch (const Registry::AutomaticKeysetManagement::CONFIGURATION_ERROR&)
    {
        throw ConfigurationError();
    }
    catch (const Registry::AutomaticKeysetManagement::INTERNAL_SERVER_ERROR&)
    {
        throw InternalServerError();
    }
    catch (const CORBA::UserException& e) // unhandled user exception
    {
        throw std::runtime_error(e._name());
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
