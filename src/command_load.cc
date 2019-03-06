/*
 * Copyright (C) 2017-2018  CZ.NIC, z. s. p. o.
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
#include <memory>

#include "src/utils.hh"
#include "src/log.hh"
#include "src/command_load.hh"

namespace Fred {
namespace Akm {


void command_load(
    const IStorage& _storage,
    const ILoader& _loader,
    const LoaderFlags& _loader_flags,
    std::unique_ptr<ILoaderOutputFilter> _filter,
    int _flags)
{
    DomainScanTaskCollection scan_tasks;
    if (_loader_flags.should_load(LoaderFlags::Flag::load_insecure))
    {
        _loader.load_insecure_tasks(scan_tasks);
    }
    if (_loader_flags.should_load(LoaderFlags::Flag::load_secure_auto))
    {
        _loader.load_secure_auto_tasks(scan_tasks);
    }
    if (_loader_flags.should_load(LoaderFlags::Flag::load_secure_noauto))
    {
        _loader.load_secure_noauto_tasks(scan_tasks);
    }

    if (_filter)
    {
        _filter->apply(scan_tasks);
        _flags |= LoadFlags::WIPE_QUEUE;
        log()->info("forcing scan queue wipe on filtered input");
    }

    if (_flags & LoadFlags::WIPE_QUEUE)
    {
        _storage.wipe_scan_queue();
    }
    if (_flags & LoadFlags::ALLOW_DUPS)
    {
        _storage.append_to_scan_queue(scan_tasks);
    }
    else
    {
        _storage.append_to_scan_queue_if_not_exists(scan_tasks);
    }
    if (_flags & LoadFlags::PRUNE)
    {
        _storage.prune_scan_queue();
    }

    log()->info("imported to database");
}


} //namespace Akm
} //namespace Fred
