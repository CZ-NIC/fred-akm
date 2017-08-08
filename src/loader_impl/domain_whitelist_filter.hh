#ifndef DOMAIN_WHITELIST_FILTER_HH_2A62B8576AF23D16826FD873281A6AFC//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define DOMAIN_WHITELIST_FILTER_HH_2A62B8576AF23D16826FD873281A6AFC

#include "src/i_loader_filter.hh"

#include <string>
#include <unordered_set>

namespace Fred {
namespace Akm {


class DomainWhitelistFilter : public ILoaderOutputFilter
{
public:
    DomainWhitelistFilter(std::string _filename);

    void apply(NameserverDomainsCollection& _collection) const;

private:
    std::unordered_set<std::string> domain_whitelist_;
};


} //namespace Akm
} //namespace Fred



#endif//DOMAIN_WHITELIST_FILTER_HH_2A62B8576AF23D16826FD873281A6AFC
