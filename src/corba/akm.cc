/*
 * Copyright (C) 2017-2018  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/corba/akm.hh"
#include "src/corba/generated/AutomaticKeysetManagement.hh"
#include "src/scan_task.hh"

#include <algorithm>
#include <exception>


namespace Fred {
namespace Akm {
namespace Corba {

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

EmailAddresses unwrap_EmailAddressSeq(Registry::AutomaticKeysetManagement::EmailAddressSeq_var _email_addresses)
{
        EmailAddresses email_addresses;
        email_addresses.reserve(_email_addresses->length());

        for (unsigned long long i = 0; i < _email_addresses->length(); ++i)
        {
            const auto& c_email_address = _email_addresses[i];
            email_addresses.emplace_back(c_email_address);
        }

        return email_addresses;
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


DomainScanTaskCollection Akm::get_nameservers_with_insecure_automatically_managed_domain_candidates() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_insecure_automatically_managed_domain_candidates();

        return unwrap_NameserverDomains(c_all_nameserver_domains, ScanType::insecure);
    }
    catch (const Registry::AutomaticKeysetManagement::INTERNAL_SERVER_ERROR& e)
    {
        throw InternalServerError();
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


DomainScanTaskCollection Akm::get_nameservers_with_secure_automatically_managed_domain_candidates() const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm = AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::NameserverDomainsSeq_var c_all_nameserver_domains = akm->get_nameservers_with_secure_automatically_managed_domain_candidates();

        return unwrap_NameserverDomains(c_all_nameserver_domains, ScanType::secure_noauto);
    }
    catch (const Registry::AutomaticKeysetManagement::INTERNAL_SERVER_ERROR& e)
    {
        throw InternalServerError();
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
    catch (const Registry::AutomaticKeysetManagement::INTERNAL_SERVER_ERROR& e)
    {
        throw InternalServerError();
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


EmailAddresses Akm::get_email_addresses_by_domain_id(unsigned long long _domain_id) const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm =
                AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));
        AkmModule::EmailAddressSeq_var c_email_addresses =
                akm->get_email_addresses_by_domain_id(_domain_id);

        return unwrap_EmailAddressSeq(c_email_addresses);
    }
    catch (const Registry::AutomaticKeysetManagement::OBJECT_NOT_FOUND& e)
    {
        throw ObjectNotFound();
    }
    catch (const Registry::AutomaticKeysetManagement::INTERNAL_SERVER_ERROR& e)
    {
        throw InternalServerError();
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


void Akm::turn_on_automatic_keyset_management_on_insecure_domain(
        unsigned long long _domain_id,
        Nsset _current_nsset,
        Keyset _new_keyset) const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm =
                AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));

        AkmModule::Nsset current_nsset = wrap_Nsset(_current_nsset);
        AkmModule::Keyset new_keyset = wrap_Keyset(_new_keyset);

        akm->turn_on_automatic_keyset_management_on_insecure_domain(_domain_id, current_nsset, new_keyset);
    }
    catch (const Registry::AutomaticKeysetManagement::OBJECT_NOT_FOUND&)
    {
        throw ObjectNotFound();
    }
    catch (const Registry::AutomaticKeysetManagement::NSSET_IS_EMPTY&)
    {
        throw NssetIsEmpty();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_NSSET_IS_EMPTY&)
    {
        throw DomainNssetIsEmpty();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_IS_INVALID&)
    {
        throw KeysetIsInvalid();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_DOES_NOT_HAVE_KEYSET&)
    {
        throw DomainAlreadyDoesNotHaveKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::NSSET_IS_DIFFERENT&)
    {
        throw NssetIsDifferent();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_HAS_KEYSET&)
    {
        throw DomainHasKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_STATE_POLICY_ERROR&)
    {
        throw DomainStatePolicyError();
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


void Akm::turn_on_automatic_keyset_management_on_secure_domain(
        unsigned long long _domain_id,
        Keyset _new_keyset) const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm =
                AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));

        AkmModule::Keyset new_keyset = wrap_Keyset(_new_keyset);

        akm->turn_on_automatic_keyset_management_on_secure_domain(_domain_id, new_keyset);
    }
    catch (const Registry::AutomaticKeysetManagement::OBJECT_NOT_FOUND&)
    {
        throw ObjectNotFound();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_IS_INVALID&)
    {
        throw KeysetIsInvalid();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_DOES_NOT_HAVE_KEYSET&)
    {
        throw DomainDoesNotHaveKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_ALREADY_HAS_AUTOMATICALLY_MANAGED_KEYSET&)
    {
        throw DomainAlreadyHasAutomaticallyManagedKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_STATE_POLICY_ERROR&)
    {
        throw DomainStatePolicyError();
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


void Akm::update_automatically_managed_keyset_of_domain(
        unsigned long long _domain_id,
        Keyset _new_keyset) const
{
    try
    {
        namespace AkmModule = Registry::AutomaticKeysetManagement;
        AkmModule::AutomaticKeysetManagementIntf_var akm =
                AkmModule::AutomaticKeysetManagementIntf::_narrow(ns_.resolve(ns_path_akm_));

        AkmModule::Keyset new_keyset = wrap_Keyset(_new_keyset);

        akm->update_automatically_managed_keyset_of_domain(_domain_id, new_keyset);
    }
    catch (const Registry::AutomaticKeysetManagement::OBJECT_NOT_FOUND&)
    {
        throw ObjectNotFound();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_IS_INVALID&)
    {
        throw KeysetIsInvalid();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_DOES_NOT_HAVE_AUTOMATICALLY_MANAGED_KEYSET&)
    {
        throw DomainDoesNotHaveAutomaticallyManagedKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_SAME_AS_DOMAIN_KEYSET&)
    {
        throw KeysetSameAsDomainKeyset();
    }
    catch (const Registry::AutomaticKeysetManagement::DOMAIN_STATE_POLICY_ERROR&)
    {
        throw DomainStatePolicyError();
    }
    catch (const Registry::AutomaticKeysetManagement::KEYSET_STATE_POLICY_ERROR&)
    {
        throw KeysetStatePolicyError();
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
