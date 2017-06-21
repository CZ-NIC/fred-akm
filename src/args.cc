#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <iostream>

#include "src/args.hh"
#include "src/utils.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


Args parse_args(int argc, char* argv[])
{
    try
    {
        namespace po = boost::program_options;

        Args args;
        auto general_args = std::make_shared<GeneralArgs>();
        auto command_load_args = std::make_shared<LoadCommandArgs>();
        args.set(general_args);

        po::options_description global("Global options");
        global.add_options()
            ("conf,c", po::value<std::string>(&general_args->config_file)->default_value("fred-akm.conf"),
             "path to configuration")
            ("help,h",
             "help message")
            ("command", po::value<std::string>(&general_args->command),
             "command to execute")
            ("command_args", po::value<std::vector<std::string>>(),
             "command arguments");

        po::positional_options_description positional;
        positional.add("command", 1).add("command_args", -1);

        po::options_description command_scan("Scan options");
        po::options_description command_load("Load options");
        command_load.add_options()
            ("input-file", po::value<std::string>(&command_load_args->input_file)->default_value(""),
             "input file which will be processed instead of data load from backend")
            ("wipe-queue", po::bool_switch(&command_load_args->wipe_queue)->default_value(false),
             "clear queue of undone tasks before load new ones")
            ("allow-dups", po::bool_switch(&command_load_args->allow_dups)->default_value(false),
             "allow duplicates tasks in scan queue")
            ("prune", po::bool_switch(&command_load_args->prune)->default_value(false),
             "prune domain with older id when importing same domain name")
            ("whitelist-file", po::value<std::string>(&command_load_args->whitelist_file)->default_value(""),
             "whitelist file with domain names (one per line) which can be imported to scan queue (enforces --wipe-queue)");
        po::options_description command_update("Update options");
        po::options_description command_notify("Notify options");

        const std::map<std::string, std::pair<po::options_description, std::shared_ptr<ArgsGroup>>> commands_mapping = {
            {"scan", {command_scan, nullptr}},
            {"load", {command_load, command_load_args}},
            {"update", {command_update, nullptr}},
            {"notify", {command_notify, nullptr}}
        };

        po::variables_map vm;

        po::parsed_options parsed = po::command_line_parser(argc, argv)
            .options(global)
            .positional(positional)
            .allow_unregistered()
            .run();

        po::store(parsed, vm);
        std::vector<std::string> unknown_opts = po::collect_unrecognized(parsed.options, po::include_positional);

        if (vm.count("help") and !vm.count("command"))
        {
            std::cout << global << std::endl;
            std::cout << "Available commands:" << std::endl;
            for (const auto& kv : commands_mapping)
            {
                std::cout << "\t" << kv.first << std::endl;
            }
            throw HelpExitHelper();
        }
        if (vm.count("command"))
        {
            // delete command name from collected unrecognized
            unknown_opts.erase(unknown_opts.begin());

            auto it = commands_mapping.find(vm["command"].as<std::string>());
            if (it == commands_mapping.end())
            {
                throw std::runtime_error("unknown command");
            }

            const auto command = it->second;
            const auto command_desc = command.first;
            const auto command_args = command.second;

            if (vm.count("help"))
            {
                std::cout << command_desc << std::endl;
                throw HelpExitHelper();
            }

            // parse so far unrecognized args with specific command parser
            po::store(
                po::command_line_parser(unknown_opts)
                    .options(command_desc)
                    .positional(po::positional_options_description())
                    .run(),
                vm
            );

            if (command_args)
            {
                args.set(command_args);
            }

            po::notify(vm);
            args.set(std::make_shared<DebugMapArgs>(variable_map_to_string_map(vm)));
        }
        else
        {
            throw std::runtime_error("unspecified command");
        }

        return args;
    }
    catch (const HelpExitHelper&)
    {
        throw;
    }
    catch (const std::exception &e)
    {
        throw ParserError(e.what());
    }
}


} // namespace Akm
} // namespace Fred
