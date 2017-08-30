#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "src/conf.hh"
#include "src/utils.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


Conf parse_conf(const std::string &_filename)
{
    std::ifstream file(_filename);
    return parse_conf(file);
}


Conf parse_conf(std::ifstream& _file)
{
    namespace po = boost::program_options;

    auto conf = Conf();
    auto nameservice_conf = std::make_shared<NameserviceConf>();
    auto database_conf = std::make_shared<DatabaseConf>();
    auto scanner_conf = std::make_shared<ScannerConf>();

    auto logging_conf = std::make_shared<LoggingConf>();
    conf.set(nameservice_conf);
    conf.set(database_conf);
    conf.set(scanner_conf);
    conf.set(logging_conf);

    auto scan_results_conf = std::make_shared<ScanResultsConf>();
    conf.set(nameservice_conf);
    conf.set(database_conf);
    conf.set(scanner_conf);
    conf.set(scan_results_conf);

    po::options_description config_file_opts("Configuration");
    config_file_opts.add_options()
        ("nameservice.host", po::value<std::string>(&nameservice_conf->host)->default_value("localhost"),
         "CORBA nameservice hostname")
        ("nameservice.port", po::value<unsigned int>(&nameservice_conf->port)->default_value(2809),
         "CORBA nameservice port")
        ("nameservice.object_path_akm", po::value<std::string>(&nameservice_conf->object_path_akm)->default_value("fred.AutomaticKeysetManagement"),
         "CORBA object location path in nameservice <context>.<object>")
        ("nameservice.object_path_mailer", po::value<std::string>(&nameservice_conf->object_path_mailer)->default_value("fred.Mailer"),
         "CORBA object location path in nameservice <context>.<object>")
        ("database.filename", po::value<std::string>(&database_conf->filename)->default_value("fred-akm.db"),
         "Sqlite database file name")
        ("scanner.tool_path", po::value<std::string>(&scanner_conf->tool_path)->default_value("/usr/bin/cdnskey-scanner"),
         "External CDNSKEY scanner tool")
        ("scanner.batch_mode", po::bool_switch(&scanner_conf->batch_mode)->default_value(false),
         "Run scanner in multiple iterations (batches)")
        ("scan_results.maximal_time_between_scan_results", po::value<unsigned long>(&scan_results_conf->maximal_time_between_scan_results)->default_value(172800),
         "Maximal time between scan results [seconds]") // 2 * 24 * 60 * 60 = 172800
        ("scan_results.minimal_scan_result_sequence_length_to_notify", po::value<unsigned long>(&scan_results_conf->minimal_scan_result_sequence_length_to_notify)->default_value(172800), // 2 * 24 * 60 * 60 = 172800
         "Minimal scan result sequence length to notify [seconds]")
        ("scan_results.minimal_scan_result_sequence_length_to_update", po::value<unsigned long>(&scan_results_conf->minimal_scan_result_sequence_length_to_update)->default_value(604800), // 7 * 24 * 60 * 60 = 604800
         "Minimal scan result sequence length to update [seconds]")
        ("scan_results.notify_from_last_iteration_only", po::value<bool>(&scan_results_conf->notify_from_last_iteration_only)->default_value(false),
         "Ignore scan_result.iteration_id")
        ("scan_results.align_to_start_of_day", po::value<bool>(&scan_results_conf->align_to_start_of_day)->default_value(false),
         "Align scan results to start of day for update. Useful if operating on daily basis.")
        ("logging.sink", po::value<std::vector<std::string>>(&logging_conf->sinks)->composing(),
         "Logging sink definition, one record per sink (available value is 'console' and 'file <file_path>' and 'syslog' followed by the logging level (available values are trace, debug, info, warning, error and critical");

    po::variables_map vm;

    po::store(po::parse_config_file(_file, config_file_opts), vm);
    po::notify(vm);
    conf.set(std::make_shared<DebugMapConf>(variable_map_to_string_map(vm)));

    return conf;
}


} // namespace Akm
} // namespace Fred
