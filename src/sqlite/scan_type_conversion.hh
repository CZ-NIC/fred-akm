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
#ifndef SCAN_TYPE_CONVERSION_HH_6DE69DD36F4616319977368533A22182//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define SCAN_TYPE_CONVERSION_HH_6DE69DD36F4616319977368533A22182

#include "src/sqlite/enum_conversion.hh"
#include "src/scan_type.hh"

#include <stdexcept>

namespace Fred {
namespace Akm {
namespace Sqlite {


inline std::string to_db_handle(const Fred::Akm::ScanType& _type)
{
    switch (_type)
    {
        case Fred::Akm::ScanType::insecure:
            return "insecure";
            break;
        case Fred::Akm::ScanType::secure_auto:
            return "secure-auto";
            break;
        case Fred::Akm::ScanType::secure_noauto:
            return "secure-noauto";
            break;
    }
    throw std::invalid_argument("Fred::Akm::ScanType value out of range");
}


template<>
inline Fred::Akm::ScanType from_db_handle<Fred::Akm::ScanType>(const std::string& _handle)
{
    if (to_db_handle(Fred::Akm::ScanType::insecure) == _handle)
    {
        return Fred::Akm::ScanType::insecure;
    }
    if (to_db_handle(Fred::Akm::ScanType::secure_auto) == _handle)
    {
        return Fred::Akm::ScanType::secure_auto;
    }
    if (to_db_handle(Fred::Akm::ScanType::secure_noauto) == _handle)
    {
        return Fred::Akm::ScanType::secure_noauto;
    }
    throw std::invalid_argument("cannot convert \"" + _handle + "\" to Fred::Akm::ScanType");
}


} // namespace Sqlite
} // namespace Akm
} // namespace Fred

#endif//SCAN_TYPE_CONVERSION_HH_6DE69DD36F4616319977368533A22182
