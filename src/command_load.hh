#ifndef COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA

#include <string>

#include "src/i_storage.hh"
#include "src/i_loader.hh"
#include "src/i_loader_filter.hh"

namespace Fred {
namespace Akm {


struct LoadFlags
{
    enum Enum
    {
        WIPE_QUEUE = (1 << 0),
        ALLOW_DUPS = (1 << 1),
        PRUNE      = (1 << 2)
    };
};


void command_load(
    const IStorage& _storage,
    const ILoader& _loader,
    std::unique_ptr<ILoaderOutputFilter> _filter, /* can be changed to filter list when needed */
    int _flags
);


} //namespace Akm
} //namespace Fred

#endif//COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA
