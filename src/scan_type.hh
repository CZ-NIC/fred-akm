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
#ifndef SCAN_TYPE_HH_ECE5FCCE435D2BCBF7E34A3C3D9C6259//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define SCAN_TYPE_HH_ECE5FCCE435D2BCBF7E34A3C3D9C6259

#include "src/from_string.hh"

#include <string>

namespace Fred {
namespace Akm {


enum class ScanType
{
    insecure,
    secure_auto,
    secure_noauto
};

std::string to_string(ScanType _scan_type);

template<> ScanType from_string(const std::string& _str);


} // namespace Akm
} // namespace Fred

#endif//SCAN_TYPE_HH_ECE5FCCE435D2BCBF7E34A3C3D9C6259
