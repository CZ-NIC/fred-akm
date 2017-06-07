#ifndef AKM_HH_C1EB76915011A3B8882D64EE0C5D9DB8//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define AKM_HH_C1EB76915011A3B8882D64EE0C5D9DB8

#include "src/corba/nameservice.hh"
#include "src/i_akm.hh"

namespace Fred {
namespace Akm {
namespace Corba {


class Akm : public IAkm
{
public:
    Akm(const Nameservice& _ns, const std::string& _ns_path_akm);

    std::vector<NameserverDomains> get_nameservers_with_automatically_managed_domain_candidates() const;

private:
    const Nameservice& ns_;
    std::string ns_path_akm_;
};


} // namespace Corba
} // namespace Akm
} // namespace Fred

#endif//AKM_HH_C1EB76915011A3B8882D64EE0C5D9DB8
