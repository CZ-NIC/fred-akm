#include <iostream>
#include <fstream>

#include "src/args.hh"
#include "src/conf.hh"
#include "src/corba/corba_context.hh"
#include "src/corba/nameservice.hh"
#include "src/sqlite/storage.hh"

#include "src/command_load.hh"

void dispatch_command_load(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    const auto& input_file = _args.get<Fred::Akm::LoadCommandArgs>()->input_file;
    if (input_file.length())
    {
        command_load(db, input_file);
    }
}


void dispatch_command_scan(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
}


void dispatch_command_notify(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
}


void dispatch_command_update(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
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

        const Fred::Akm::Corba::CorbaContext cctx(argc, argv, nameservice_conf->host, nameservice_conf->port);

        typedef std::function<void(Fred::Akm::Corba::CorbaContext, Fred::Akm::Args, Fred::Akm::Conf)> CommandDispatchFunc;
        const std::map<std::string, CommandDispatchFunc> command_dispatch = {
            {"load", &dispatch_command_load},
            {"scan", &dispatch_command_scan},
            {"notify", &dispatch_command_notify},
            {"update", &dispatch_command_update},
        };

        command_dispatch.at(general_args->command)(cctx, args, conf);
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
