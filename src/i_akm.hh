#ifndef I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F

#include "src/keyset.hh"
#include "src/nameserver_domains.hh"
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


struct NssetInvalid : AkmException
{
};


struct KeysetInvalid : AkmException
{
};


struct NssetDiffers : AkmException
{
};


struct DomainHasOtherKeyset : AkmException
{
};


struct DomainStatePolicyError : AkmException
{
};


struct KeysetStatePolicyError : AkmException
{
};


struct SystemRegistratorNotFound : AkmException
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
    virtual NameserverDomainsCollection get_nameservers_with_automatically_managed_domain_candidates() const = 0;

    virtual NameserverDomainsCollection get_nameservers_with_automatically_managed_domains() const = 0;

    virtual std::vector<std::string> get_nsset_notification_emails_by_domain_id(unsigned long long domain_id) const = 0;

    virtual void update_domain_automatic_keyset(unsigned long long domain_id, Nsset current_nsset, Keyset new_keyset) const = 0;
};


} // namespace Akm
} // namespace Fred

#endif//I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F
