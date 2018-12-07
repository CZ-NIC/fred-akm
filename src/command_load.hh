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
#ifndef COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA

#include <string>

#include "src/i_storage.hh"
#include "src/i_loader.hh"
#include "src/i_loader_filter.hh"

namespace Fred {
namespace Akm {


struct LoadFlags
{
    enum Enum
    {
        WIPE_QUEUE = (1 << 0),
        ALLOW_DUPS = (1 << 1),
        PRUNE      = (1 << 2)
    };
};


void command_load(
    const IStorage& _storage,
    const ILoader& _loader,
    const LoaderFlags& _loader_flags,
    std::unique_ptr<ILoaderOutputFilter> _filter, /* can be changed to filter list when needed */
    int _flags
);


} //namespace Akm
} //namespace Fred

#endif//COMMAND_LOAD_HH_EE8D80C700F7A58D45939FF957C573BA
