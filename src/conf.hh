#ifndef CONF_HH_97B80A5E961D13153BCB013A588CA5CF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CONF_HH_97B80A5E961D13153BCB013A588CA5CF

#include <string>

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


Conf parse_conf(const std::string &_filename);
Conf parse_conf(std::ifstream &_file);


} // namespace Akm
} // namespace Fred

#endif//CONF_HH_97B80A5E961D13153BCB013A588CA5CF
