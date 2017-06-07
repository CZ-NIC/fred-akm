#ifndef I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB

#include <string>
#include <vector>

#include "src/nameserver_domains.hh"

namespace Fred {
namespace Akm {


class IStorage
{
public:
    virtual void new_scan_tasks(const std::vector<NameserverDomains>& _data) const = 0;

    virtual void append_scan_tasks(const std::vector<NameserverDomains>& _data) const = 0;
};


} // namespace Fred
} // namespace Akm

#endif//I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB
