/*
 * Copyright (C) 2017  CZ.NIC, z. s. p. o.
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
#ifndef ARGS_HH_F100B1BE437CD3756EA6F23E0C173BEE//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define ARGS_HH_F100B1BE437CD3756EA6F23E0C173BEE

#include <stdexcept>
#include <memory>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

#include "src/type_instance_map.hh"

namespace Fred {
namespace Akm {


struct ArgsGroup
{
    virtual ~ArgsGroup() {}
};

struct ArgsGroupNotFound : public std::exception
{
    const char* what() const noexcept { return "argument group not found"; }
};

class Args : public TypeInstanceMap<ArgsGroup, ArgsGroupNotFound> { };


struct GeneralArgs : public ArgsGroup
{
    std::string config_file;
    std::string command;
};

struct LoadCommandArgs : public ArgsGroup
{
    std::string input_file;
    bool wipe_queue;
    bool allow_dups;
    bool prune;
    bool no_insecure;
    bool no_secure_auto;
    bool no_secure_noauto;
    std::string whitelist_file;
};

struct NotifyCommandArgs : public ArgsGroup
{
    bool dry_run;
    bool fake_contact_emails;
};

struct UpdateCommandArgs : public ArgsGroup
{
    bool dry_run;
    bool fake_contact_emails;
};

struct DebugMapArgs : public ArgsGroup
{
    DebugMapArgs(const std::unordered_map<std::string, std::string>& _map)
        : debug_map(_map) { }

    std::unordered_map<std::string, std::string> debug_map;
};


struct HelpExitHelper : std::exception
{
    const char* what() const noexcept { return "help called"; }
};

struct ParserError : std::runtime_error
{
    ParserError(const char *str) : std::runtime_error(str) { }
};


Args parse_args(int argc, char* argv[]);


} // namespace Fred
} // namespace Akm

#endif//ARGS_HH_F100B1BE437CD3756EA6F23E0C173BEE
