#ifndef ARGS_HH_F100B1BE437CD3756EA6F23E0C173BEE//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define ARGS_HH_F100B1BE437CD3756EA6F23E0C173BEE

#include <stdexcept>
#include <memory>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

#include "type_instance_map.hh"

namespace Fred {
namespace Akm {


struct ArgsGroup
{
    virtual ~ArgsGroup() {}
};

struct ArgsGroupNotFound : public std::exception
{
    const char* what() const noexcept { return "argument group not found"; }
};

class Args : public TypeInstanceMap<ArgsGroup, ArgsGroupNotFound> { };


struct GeneralArgs : public ArgsGroup
{
    std::string config_file;
    std::string command;
};

struct LoadCommandArgs : public ArgsGroup
{
    std::string input_file;
    bool wipe_queue;
    bool allow_dups;
    bool prune;
    std::string whitelist_file;
};

struct DebugMapArgs : public ArgsGroup
{
    DebugMapArgs(const std::unordered_map<std::string, std::string>& _map)
        : debug_map(_map) { }

    std::unordered_map<std::string, std::string> debug_map;
};


struct HelpExitHelper : std::exception
{
    const char* what() const noexcept { return "help called"; }
};

struct ParserError : std::runtime_error
{
    ParserError(const char *str) : std::runtime_error(str) { }
};


Args parse_args(int argc, char* argv[]);


} // namespace Fred
} // namespace Akm

#endif//ARGS_HH_F100B1BE437CD3756EA6F23E0C173BEE
