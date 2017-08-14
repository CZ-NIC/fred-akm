#ifndef I_LOADER_HH_F9A17F4C1EBB4F9A00228541A3198598//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_LOADER_HH_F9A17F4C1EBB4F9A00228541A3198598

#include "src/scan_task.hh"

namespace Fred {
namespace Akm {


class ILoader
{
public:
    virtual void load_domains(DomainScanTaskCollection& _scan_tasks) const = 0;
};


} //namespace Akm
} //namespace Fred

#endif//I_LOADER_HH_F9A17F4C1EBB4F9A00228541A3198598
