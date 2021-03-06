/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <fstream>

#include "src/args.hh"
#include "src/conf.hh"
#include "src/corba/corba_context.hh"
#include "src/corba/nameservice.hh"
#include "src/corba/akm.hh"
#include "src/corba/mailer.hh"
#include "src/sqlite/storage.hh"
#include "src/cdnskey_scanner_impl/cdnskey_scanner_impl.hh"
#include "src/loader_impl/file.hh"
#include "src/loader_impl/backend.hh"
#include "src/loader_impl/domain_whitelist_filter.hh"
#include "src/log.hh"

#include "src/command_clean.hh"
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

    Fred::Akm::LoaderFlags loader_flags;
    if (load_args->no_insecure)
    {
        loader_flags.disable_load(Fred::Akm::LoaderFlags::Flag::load_insecure);
    }
    if (load_args->no_secure_auto)
    {
        loader_flags.disable_load(Fred::Akm::LoaderFlags::Flag::load_secure_auto);
    }
    if (load_args->no_secure_noauto)
    {
        loader_flags.disable_load(Fred::Akm::LoaderFlags::Flag::load_secure_noauto);
    }

    std::unique_ptr<Fred::Akm::ILoaderOutputFilter> filter;
    if (whitelist_file.length())
    {
        filter = std::unique_ptr<Fred::Akm::ILoaderOutputFilter>(new Fred::Akm::DomainWhitelistFilter(whitelist_file));
    }

    if (input_file.length())
    {
        command_load(db, Fred::Akm::FileLoader(input_file), loader_flags, std::move(filter), load_flags);
    }
    else
    {
        auto akm_backend = Fred::Akm::Corba::Akm(_cctx.get_nameservice(), _conf.get<Fred::Akm::NameserviceConf>()->object_path_akm);
        command_load(db, Fred::Akm::BackendLoader(akm_backend), loader_flags, std::move(filter), load_flags);
    }
}


void dispatch_command_scan(
    const Fred::Akm::Corba::CorbaContext&,
    const Fred::Akm::Args&,
    const Fred::Akm::Conf& _conf)
{
    const auto& scanner_tool_path = _conf.get<Fred::Akm::ScannerConf>()->tool_path;
    const auto& scanner_batch_mode = _conf.get<Fred::Akm::ScannerConf>()->batch_mode;
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    Fred::Akm::ExternalCdnskeyScannerImpl scanner(scanner_tool_path);
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
    const auto align_to_start_of_day = _conf.get<Fred::Akm::ScanResultsConf>()->align_to_start_of_day;
    const auto dry_run = _args.get<Fred::Akm::NotifyCommandArgs>()->dry_run;
    const auto fake_contact_emails = _args.get<Fred::Akm::NotifyCommandArgs>()->fake_contact_emails;

    command_notify(
            db,
            akm_backend,
            mailer_backend,
            maximal_time_between_scan_results,
            minimal_scan_result_sequence_length_to_notify,
            notify_from_last_iteration_only,
            align_to_start_of_day,
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
    const auto align_to_start_of_day = _conf.get<Fred::Akm::ScanResultsConf>()->align_to_start_of_day;
    const auto dry_run = _args.get<Fred::Akm::UpdateCommandArgs>()->dry_run;
    const auto fake_contact_emails = _args.get<Fred::Akm::UpdateCommandArgs>()->fake_contact_emails;

    command_update(
            db,
            akm_backend,
            mailer_backend,
            maximal_time_between_scan_results,
            minimal_scan_result_sequence_length_to_update,
            align_to_start_of_day,
            dry_run,
            fake_contact_emails);
}


void dispatch_command_clean(
    const Fred::Akm::Corba::CorbaContext&,
    const Fred::Akm::Args&,
    const Fred::Akm::Conf& _conf)
{
    Fred::Akm::Sqlite::SqliteStorage db(_conf.get<Fred::Akm::DatabaseConf>()->filename);
    const auto maximal_time_between_scan_results = _conf.get<Fred::Akm::ScanResultsConf>()->maximal_time_between_scan_results;
    const auto minimal_scan_result_sequence_length_to_update = _conf.get<Fred::Akm::ScanResultsConf>()->minimal_scan_result_sequence_length_to_update;
    const auto align_to_start_of_day = _conf.get<Fred::Akm::ScanResultsConf>()->align_to_start_of_day;

    command_clean(
            db,
            maximal_time_between_scan_results,
            minimal_scan_result_sequence_length_to_update,
            align_to_start_of_day);
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
            {"clean", &dispatch_command_clean},
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
