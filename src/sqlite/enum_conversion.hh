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
#ifndef ENUM_CONVERSION_HH_04D16325B82D363D0CF35195417A0A8C//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define ENUM_CONVERSION_HH_04D16325B82D363D0CF35195417A0A8C

#include <string>

namespace Fred {
namespace Akm {
namespace Sqlite {


template<class T>
T from_db_handle(const std::string& _handle);


} // namespace Sqlite
} // namespace Akm
} // namespace Fred

#endif//ENUM_CONVERSION_HH_04D16325B82D363D0CF35195417A0A8C
