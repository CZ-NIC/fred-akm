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
#ifndef I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F

#include "src/keyset.hh"
#include "src/scan_task.hh"
#include "src/nsset.hh"

#include <exception>
#include <map>
#include <string>

namespace Fred {
namespace Akm {


struct AkmException
    : virtual std::exception
{
    virtual const char* what() const noexcept = 0;
};


/**
 * Requested object was not found.
 * Requested object could have been deleted or set into inappropriate state.
 */
struct ObjectNotFound
    : AkmException
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    virtual const char* what() const noexcept
    {
        return "registry object with specified ID does not exist";
    }
};


struct NssetIsEmpty
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "current_nsset is empty";
    }
};


struct DomainNssetIsEmpty
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain nsset is empty";
    }
};


struct KeysetIsInvalid
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "current_keyset is invalid";
    }
};


struct NssetIsDifferent
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "current_nsset differs";
    }
};


struct DomainHasKeyset
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain has keyset (domain is not insecure)";
    }
};


struct DomainDoesNotHaveKeyset
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain does not have a keyset (domain is not secure)";
    }
};


struct DomainAlreadyDoesNotHaveKeyset
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain already does not have a keyset";
    }
};


struct DomainDoesNotHaveAutomaticallyManagedKeyset
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain does not have an automatically managed keyset";
    }
};


struct DomainAlreadyHasAutomaticallyManagedKeyset
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain already has an automatically managed keyset";
    }
};


struct KeysetSameAsDomainKeyset
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "the keyset is the same as the current keyset of the domain";
    }
};


struct DomainStatePolicyError
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "domain state prevents action";
    }
};


struct KeysetStatePolicyError
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "keyset state prevents action";
    }
};


struct ConfigurationError
    : AkmException
{
    virtual const char* what() const noexcept
    {
        return "configuration error";
    }
};


/**
 * Internal server error.
 * Unexpected failure, requires maintenance.
 */
struct InternalServerError
    : AkmException
{
    /**
     * Returns failure description.
     * @return string with the general cause of the current error.
     */
    virtual const char* what() const noexcept
    {
        return "internal server error";
    }
};


class IAkm
{
public:
    virtual DomainScanTaskCollection get_nameservers_with_insecure_automatically_managed_domain_candidates() const = 0;

    virtual DomainScanTaskCollection get_nameservers_with_secure_automatically_managed_domain_candidates() const = 0;

    virtual DomainScanTaskCollection get_nameservers_with_automatically_managed_domains() const = 0;

    virtual std::vector<std::string> get_email_addresses_by_domain_id(unsigned long long _domain_id) const = 0;

    virtual void turn_on_automatic_keyset_management_on_insecure_domain(
            unsigned long long _domain_id,
            Nsset _current_nsset,
            Keyset _new_keyset) const = 0;

    virtual void turn_on_automatic_keyset_management_on_secure_domain(
            unsigned long long _domain_id,
            Keyset _new_keyset) const = 0;

    virtual void update_automatically_managed_keyset_of_domain(
            unsigned long long _domain_id,
            Keyset _new_keyset) const = 0;
};


} // namespace Akm
} // namespace Fred

#endif//I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F
