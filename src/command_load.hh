#ifndef COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA

#include <string>

#include "src/i_storage.hh"

namespace Fred {
namespace Akm {


void command_load(const IStorage& _storage, const std::string& _filename);


} //namespace Akm
} //namespace Fred

#endif//COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA
