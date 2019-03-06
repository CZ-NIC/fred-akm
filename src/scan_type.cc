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
#include "src/scan_type.hh"
#include "src/sqlite/scan_type_conversion.hh"

namespace Fred {
namespace Akm {


std::string to_string(ScanType _scan_type)
{
    /* for now use same conversion as to db handle */
    return Sqlite::to_db_handle(_scan_type);
}


template<>
ScanType from_string(const std::string& _str)
{
    /* for now use same conversion as from db handle */
    return Sqlite::from_db_handle<ScanType>(_str);
}


} // namespace Akm
} // namespace Fred


