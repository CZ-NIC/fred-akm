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
#ifndef OUTPUT_PARSER_HH_DDEC7B4468DE682C3826F9CD09E6C08B//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define OUTPUT_PARSER_HH_DDEC7B4468DE682C3826F9CD09E6C08B

#include "src/i_scanner.hh"

#include <string>


namespace Fred {
namespace Akm {


class ScanResultParser
{
public:
    ScanResult parse(const std::string& _line) const;

    std::vector<ScanResult> parse_multi(std::string& _multi_line) const;
};


} // namespace Akm
} // namespace Fred

#endif//OUTPUT_PARSER_HH_DDEC7B4468DE682C3826F9CD09E6C08B
