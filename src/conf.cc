#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "src/conf.hh"

namespace Fred {
namespace Akm {


Conf parse_conf(const std::string &_filename)
{
    auto file = std::ifstream(_filename);
    return parse_conf(file);
}


Conf parse_conf(std::ifstream& _file)
{
    namespace po = boost::program_options;

    auto conf = Conf();
    auto nameservice_conf = std::make_shared<NameserviceConf>();
    conf.set(nameservice_conf);

    po::options_description config_file_opts("Configuration");
    config_file_opts.add_options()
        ("nameservice.host", po::value<std::string>(&nameservice_conf->host)->default_value("localhost"),
         "CORBA nameservice hostname")
        ("nameservice.port", po::value<std::string>(&nameservice_conf->port)->default_value("2809"),
         "CORBA nameservice port")
        ("nameservice.object_path", po::value<std::string>(&nameservice_conf->object_path)->default_value("fred.AutomaticKeysetManagement"),
         "CORBA object location path in nameservice <context>.<object>");

    po::variables_map vm;

    po::store(po::parse_config_file(_file, config_file_opts), vm);
    po::notify(vm);

    return conf;
}


} // namespace Akm
} // namespace Fred
