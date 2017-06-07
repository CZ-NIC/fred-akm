#ifndef NAMESERVER_DOMAINS_HH_BD50E84D81AF2A84043E22FA4923D539//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define NAMESERVER_DOMAINS_HH_BD50E84D81AF2A84043E22FA4923D539

#include <string>
#include <vector>

namespace Fred {
namespace Akm {


struct Domain
{
    Domain(unsigned long long _id, const std::string& _fqdn)
        : id(_id), fqdn(_fqdn)
    {
    }

    unsigned long long id;
    std::string fqdn;
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


} // namespace Fred
} // namespace Akm


#endif//NAMESERVER_DOMAINS_HH_BD50E84D81AF2A84043E22FA4923D539
