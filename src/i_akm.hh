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


struct AkmException : virtual std::exception
{
};


struct ObjectNotFound : AkmException
{
};


struct NssetIsInvalid : AkmException
{
};


struct KeysetIsInvalid : AkmException
{
};


struct NssetIsEmpty : AkmException
{
};


struct NssetIsDifferent : AkmException
{
};


struct DomainNssetIsEmpty : AkmException
{
};


struct DomainHasKeyset : AkmException
{
};


struct DomainDoesNotHaveKeyset : AkmException
{
};


struct DomainAlreadyHasAutomaticallyManagedKeyset : AkmException
{
};


struct DomainStatePolicyError : AkmException
{
};


struct KeysetStatePolicyError : AkmException
{
};


struct ConfigurationError : AkmException
{
};


struct InternalServerError : AkmException
{
};


class IAkm
{
public:
    virtual DomainScanTaskCollection get_nameservers_with_insecure_automatically_managed_domain_candidates() const = 0;

    virtual DomainScanTaskCollection get_nameservers_with_automatically_managed_domains() const = 0;

    virtual DomainScanTaskCollection get_nameservers_with_secure_automatically_managed_domain_candidates() const = 0;

    virtual std::vector<std::string> get_email_addresses_by_domain_id(unsigned long long domain_id) const = 0;

    virtual void turn_on_automatic_keyset_management_on_insecure_domain(
        unsigned long long domain_id,
        Nsset current_nsset,
        Keyset new_keyset) const = 0;

    virtual void turn_on_automatic_keyset_management_on_secure_domain(
        unsigned long long domain_id,
        Keyset new_keyset) const = 0;

    virtual void update_automatically_managed_keyset_of_domain(
        unsigned long long domain_id,
        Keyset new_keyset) const = 0;
};


} // namespace Akm
} // namespace Fred

#endif//I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F
