#ifndef CONF_HH_97B80A5E961D13153BCB013A588CA5CF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CONF_HH_97B80A5E961D13153BCB013A588CA5CF

#include <string>
#include <vector>
#include <unordered_map>

#include "type_instance_map.hh"

namespace Fred {
namespace Akm {


struct ConfSection
{
    virtual ~ConfSection() { };
};

struct ConfSectionNotFound : public std::exception
{
    const char* what() const noexcept { return "configuration section not found"; }
};

class Conf : public TypeInstanceMap<ConfSection, ConfSectionNotFound> { };


struct NameserviceConf : public ConfSection
{
    std::string host;
    unsigned int port;
    std::string object_path;
};

struct DatabaseConf : public ConfSection
{
    std::string filename;
};

struct ScannerConf : public ConfSection
{
    std::string tool_path;
};

struct LoggingConf : public ConfSection
{
    std::vector<std::string> sinks;
    std::string level;
};

struct DebugMapConf : public ConfSection
{
    DebugMapConf(const std::unordered_map<std::string, std::string>& _map)
        : debug_map(_map) { }

    std::unordered_map<std::string, std::string> debug_map;
};


Conf parse_conf(const std::string &_filename);
Conf parse_conf(std::ifstream &_file);


} // namespace Akm
} // namespace Fred

#endif//CONF_HH_97B80A5E961D13153BCB013A588CA5CF
