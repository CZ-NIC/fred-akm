#ifndef BACKEND_HH_DF70BFC9D12CF33FF8BAAB1131ABA697//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define BACKEND_HH_DF70BFC9D12CF33FF8BAAB1131ABA697

#include "src/i_loader.hh"
#include "src/i_akm.hh"

namespace Fred {
namespace Akm {


class BackendLoader : public ILoader
{
public:
    BackendLoader(const IAkm& _akm_backend);

    void load_insecure_tasks(DomainScanTaskCollection& _scan_tasks) const;

    void load_secure_auto_tasks(DomainScanTaskCollection& _scan_tasks) const;

    void load_secure_noauto_tasks(DomainScanTaskCollection& _scan_tasks) const;

private:
    const IAkm& akm_backend_;
};


} //namespace Akm
} //namespace Fred

#endif//BACKEND_HH_DF70BFC9D12CF33FF8BAAB1131ABA697
