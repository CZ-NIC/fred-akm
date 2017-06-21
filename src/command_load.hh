#ifndef COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA

#include <string>

#include "src/i_storage.hh"
#include "src/i_akm.hh"

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


void command_load(const IStorage& _storage, const std::string& _filename, const std::string& _whitelist_filename, int _flags);


void command_load(const IStorage& _storage, const IAkm& _backend, const std::string& _whitelist_filename, int _flags);


} //namespace Akm
} //namespace Fred

#endif//COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA
