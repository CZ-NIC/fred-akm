#ifndef I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F

#include <string>
#include <map>

#include "src/nameserver_domains.hh"

namespace Fred {
namespace Akm {


class IAkm
{
public:
    virtual NameserverDomainsCollection get_nameservers_with_automatically_managed_domain_candidates() const = 0;

    virtual NameserverDomainsCollection get_nameservers_with_automatically_managed_domains() const = 0;

    virtual std::vector<std::string> get_nsset_notification_emails_by_domain_id(unsigned long long domain_id) const = 0;
};


} // namespace Akm
} // namespace Fred

#endif//I_AKM_HH_EF26886C00D8568723C7861B5F04FF1F
