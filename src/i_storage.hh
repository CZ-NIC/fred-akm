#ifndef I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB

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


class IStorage
{
public:
    virtual void new_scan_tasks(const std::vector<NameserverDomains>& _data) const = 0;

    virtual void append_scan_tasks(const std::vector<NameserverDomains>& _data) const = 0;
};


} // namespace Fred
} // namespace Akm

#endif//I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB
