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
#ifndef UTILS_HH_D4018E84191F90068B09B1FB56C850C7//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define UTILS_HH_D4018E84191F90068B09B1FB56C850C7

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/program_options.hpp>

namespace Fred {
namespace Akm {


void split_on(const std::string& _in_string, const char delimiter, std::vector<std::string>& _out_tokens);

std::unordered_map<std::string, std::string> variable_map_to_string_map(
    const boost::program_options::variables_map& _map
);


} // namespace Akm
} // namespace Fred

#endif//UTILS_HH_D4018E84191F90068B09B1FB56C850C7
