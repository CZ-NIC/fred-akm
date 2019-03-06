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
#ifndef CONF_HH_97B80A5E961D13153BCB013A588CA5CF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CONF_HH_97B80A5E961D13153BCB013A588CA5CF

#include <string>
#include <vector>
#include <unordered_map>

#include "src/type_instance_map.hh"

namespace Fred {
namespace Akm {


struct ConfSection
{
    virtual ~ConfSection() { };
};

struct ConfSectionNotFound : public std::exception
{
    const char* what() const noexcept { return "configuration section not found"; }
};

class Conf : public TypeInstanceMap<ConfSection, ConfSectionNotFound> { };


struct NameserviceConf : public ConfSection
{
    std::string host;
    unsigned int port;
    std::string object_path_akm;
    std::string object_path_mailer;
};

struct DatabaseConf : public ConfSection
{
    std::string filename;
};

struct ScannerConf : public ConfSection
{
    std::string tool_path;
    bool batch_mode;
};

struct LoggingConf : public ConfSection
{
    std::vector<std::string> sinks;
};

struct DebugMapConf : public ConfSection
{
    DebugMapConf(const std::unordered_map<std::string, std::string>& _map)
        : debug_map(_map) { }

    std::unordered_map<std::string, std::string> debug_map;
};


struct ScanResultsConf : public ConfSection
{
    unsigned long maximal_time_between_scan_results;
    unsigned long minimal_scan_result_sequence_length_to_notify;
    unsigned long minimal_scan_result_sequence_length_to_update;
    bool notify_from_last_iteration_only;
    bool align_to_start_of_day;
};


Conf parse_conf(const std::string &_filename);
Conf parse_conf(std::ifstream &_file);


} // namespace Akm
} // namespace Fred

#endif//CONF_HH_97B80A5E961D13153BCB013A588CA5CF
