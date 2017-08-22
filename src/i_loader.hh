#ifndef I_LOADER_HH_F9A17F4C1EBB4F9A00228541A3198598//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_LOADER_HH_F9A17F4C1EBB4F9A00228541A3198598

#include "src/scan_task.hh"

namespace Fred {
namespace Akm {


class LoaderFlags
{
private:
    int flags_;

public:
    enum class Flag : int
    {
        load_insecure      = (1 << 0),
        load_secure_auto   = (1 << 1),
        load_secure_noauto = (1 << 2)
    };


    LoaderFlags() : flags_(
        static_cast<int>(Flag::load_insecure)
        | static_cast<int>(Flag::load_secure_auto)
        | static_cast<int>(Flag::load_secure_noauto)
    )
    {
    }

    void disable_load(LoaderFlags::Flag _flag)
    {
        flags_ &= ~static_cast<int>(_flag);
    }

    bool should_load(LoaderFlags::Flag _flag) const
    {
        return flags_ & static_cast<int>(_flag);
    }
};


class ILoader
{
public:
    virtual void load_insecure_tasks(DomainScanTaskCollection& _scan_tasks) const = 0;

    virtual void load_secure_auto_tasks(DomainScanTaskCollection& _scan_tasks) const = 0;

    virtual void load_secure_noauto_tasks(DomainScanTaskCollection& _scan_tasks) const = 0;
};


} //namespace Akm
} //namespace Fred

#endif//I_LOADER_HH_F9A17F4C1EBB4F9A00228541A3198598
