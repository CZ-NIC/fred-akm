#ifndef NAMESERVER_DOMAINS_HH_BD50E84D81AF2A84043E22FA4923D539//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define NAMESERVER_DOMAINS_HH_BD50E84D81AF2A84043E22FA4923D539

#include <unordered_map>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


struct Domain
{
    Domain(unsigned long long _id, const std::string& _fqdn, bool _has_keyset)
        : id(_id), fqdn(_fqdn), has_keyset(_has_keyset)
    {
    }

    unsigned long long id;
    std::string fqdn;
    bool has_keyset;
};

struct NameserverDomains
{
    NameserverDomains()
        : nameserver(std::string()), nameserver_domains()
    {
    }

    NameserverDomains(const std::string& _nameserver, const std::vector<Domain>& _nameserver_domains)
        : nameserver(_nameserver), nameserver_domains(_nameserver_domains)
    {
    }

    std::string nameserver;
    std::vector<Domain> nameserver_domains;
};

typedef std::unordered_map<std::string, NameserverDomains> NameserverDomainsCollection;

} // namespace Fred
} // namespace Akm


#endif//NAMESERVER_DOMAINS_HH_BD50E84D81AF2A84043E22FA4923D539
