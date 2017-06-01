#include <iostream>
#include <fstream>

#include "src/args.hh"
#include "src/conf.hh"

#include "src/corba/nameservice.hh"


void command_dispatcher(const Fred::Akm::Args& _args, const Fred::Akm::Conf& _conf)
{
    const auto& command = _args.get<Fred::Akm::GeneralArgs>()->command;
    if (command == "load")
    {
    }
    else if (command == "scan")
    {
    }
    else if (command == "notify")
    {
    }
    else if (command == "update")
    {
    }
}


int main(int argc, char* argv[])
{
    try
    {
        const auto args = Fred::Akm::parse_args(argc, argv);

        const auto general_args = args.get<Fred::Akm::GeneralArgs>();
        std::cout << "args.general.config_file = " << general_args->config_file << std::endl;
        std::cout << "args.general.command = " << general_args->command << std::endl;

        auto config_file = std::ifstream(general_args->config_file);
        if (!config_file.is_open())
        {
            throw std::runtime_error("config file not found");
        }

        const auto conf = Fred::Akm::parse_conf(config_file);

        const auto nameservice_conf = conf.get<Fred::Akm::NameserviceConf>();
        std::cout << "nameservice.host = " << nameservice_conf->host << std::endl;
        std::cout << "nameservice.port = " << nameservice_conf->port << std::endl;
        std::cout << "nameservice.object_path = " << nameservice_conf->object_path << std::endl;

        command_dispatcher(args, conf);
        return 0;
    }
    catch (const Fred::Akm::HelpExitHelper&)
    {
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown error occured" << std::endl;
        return -2;
    }
}
