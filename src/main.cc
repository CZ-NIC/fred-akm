#include <iostream>
#include <fstream>

#include "src/args.hh"
#include "src/conf.hh"
#include "src/corba/corba_context.hh"
#include "src/corba/nameservice.hh"
#include "src/corba/akm.hh"
#include "src/corba/mailer.hh"
#include "src/sqlite/storage.hh"
#include "src/external_scanner.hh"
#include "src/log.hh"

#include "src/command_load.hh"
#include "src/command_notify.hh"
#include "src/command_scan.hh"
#include "src/command_update.hh"


void debug_input_params(
    const std::unordered_map<std::string, std::string>& _map,
    const std::string& _prefix = ""
)
{
    for (const auto kv : _map)
    {
        std::string prefix;
        if (_prefix.size())
        {
            prefix.append(_prefix + ".");
        }
        Fred::Akm::log()->debug("{}{}: {}", prefix, kv.first, kv.second);
    }
};


void dispatch_command_load(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    const auto load_args = _args.get<Fred::Akm::LoadCommandArgs>();
    const auto& input_file = load_args->input_file;
    const auto& whitelist_file = load_args->whitelist_file;
    int load_flags = 0;
    if (load_args->wipe_queue)
    {
        load_flags |= Fred::Akm::LoadFlags::WIPE_QUEUE;
    }
    if (load_args->allow_dups)
    {
        load_flags |= Fred::Akm::LoadFlags::ALLOW_DUPS;
    }
    if (load_args->prune)
    {
        load_flags |= Fred::Akm::LoadFlags::PRUNE;
    }
    if (input_file.length())
    {
        command_load(db, input_file, whitelist_file, load_flags);
    }
    else
    {
        auto akm_backend = Fred::Akm::Corba::Akm(_cctx.get_nameservice(), _conf.get<Fred::Akm::NameserviceConf>()->object_path_akm);
        command_load(db, akm_backend, whitelist_file, load_flags);
    }
}


void dispatch_command_scan(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
    const auto& scanner_tool_path = _conf.get<Fred::Akm::ScannerConf>()->tool_path;
    const auto& scanner_batch_mode = _conf.get<Fred::Akm::ScannerConf>()->batch_mode;
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    Fred::Akm::ExternalScannerTool scanner(scanner_tool_path);
    command_scan(db, scanner, scanner_batch_mode);
}


void dispatch_command_notify(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    auto akm_backend = Fred::Akm::Corba::Akm(_cctx.get_nameservice(), _conf.get<Fred::Akm::NameserviceConf>()->object_path_akm);
    auto mailer_backend = Fred::Akm::Corba::Mailer(_cctx.get_nameservice(), _conf.get<Fred::Akm::NameserviceConf>()->object_path_mailer);
    const auto maximal_time_between_scan_results = _conf.get<Fred::Akm::ScanResultsConf>()->maximal_time_between_scan_results;
    const auto minimal_scan_result_sequence_length_to_notify = _conf.get<Fred::Akm::ScanResultsConf>()->minimal_scan_result_sequence_length_to_notify;
    const auto notify_from_last_iteration_only = _conf.get<Fred::Akm::ScanResultsConf>()->notify_from_last_iteration_only;
    const auto dry_run = _args.get<Fred::Akm::NotifyCommandArgs>()->dry_run;
    const auto fake_contact_emails = _args.get<Fred::Akm::NotifyCommandArgs>()->fake_contact_emails;

    command_notify(
            db,
            akm_backend,
            mailer_backend,
            maximal_time_between_scan_results,
            minimal_scan_result_sequence_length_to_notify,
            notify_from_last_iteration_only,
            dry_run,
            fake_contact_emails);
}


void dispatch_command_update(
    const Fred::Akm::Corba::CorbaContext& _cctx,
    const Fred::Akm::Args& _args,
    const Fred::Akm::Conf& _conf)
{
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    auto akm_backend = Fred::Akm::Corba::Akm(_cctx.get_nameservice(), _conf.get<Fred::Akm::NameserviceConf>()->object_path_akm);
    auto mailer_backend = Fred::Akm::Corba::Mailer(_cctx.get_nameservice(), _conf.get<Fred::Akm::NameserviceConf>()->object_path_mailer);
    const auto maximal_time_between_scan_results = _conf.get<Fred::Akm::ScanResultsConf>()->maximal_time_between_scan_results;
    const auto minimal_scan_result_sequence_length_to_update = _conf.get<Fred::Akm::ScanResultsConf>()->minimal_scan_result_sequence_length_to_update;
    const auto notify_from_last_iteration_only = _conf.get<Fred::Akm::ScanResultsConf>()->notify_from_last_iteration_only;
    const auto dry_run = _args.get<Fred::Akm::UpdateCommandArgs>()->dry_run;
    const auto fake_contact_emails = _args.get<Fred::Akm::UpdateCommandArgs>()->fake_contact_emails;

    command_update(
            db,
            akm_backend,
            mailer_backend,
            maximal_time_between_scan_results,
            minimal_scan_result_sequence_length_to_update,
            dry_run,
            fake_contact_emails);
}


int main(int argc, char* argv[])
{
    try
    {
        const auto args = Fred::Akm::parse_args(argc, argv);

        const auto general_args = args.get<Fred::Akm::GeneralArgs>();
        std::ifstream config_file(general_args->config_file); // = constructor is deleted
        if (!config_file.is_open())
        {
            throw std::runtime_error("config file not found");
        }
        const auto conf = Fred::Akm::parse_conf(config_file);

        const auto logging_conf = conf.get<Fred::Akm::LoggingConf>();
        Fred::Akm::setup_logging(logging_conf->sinks);

        debug_input_params(args.get<Fred::Akm::DebugMapArgs>()->debug_map, "args");
        debug_input_params(conf.get<Fred::Akm::DebugMapConf>()->debug_map, "conf");


        const auto nameservice_conf = conf.get<Fred::Akm::NameserviceConf>();
        const char* options[][2] = { { "nativeCharCodeSet", "UTF-8" }, { 0, 0 } };
        const Fred::Akm::Corba::CorbaContext cctx(argc, argv, nameservice_conf->host, nameservice_conf->port, options);

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
        Fred::Akm::log()->error(e.what());
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown error occured" << std::endl;
        Fred::Akm::log()->error("unknown error occured");
        return -2;
    }
}
