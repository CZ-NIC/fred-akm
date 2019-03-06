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
#ifndef FROM_STRING_HH_3AC40C9B47AC09214CE546C9737AEA9D//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define FROM_STRING_HH_3AC40C9B47AC09214CE546C9737AEA9D

#include <string>

namespace Fred {
namespace Akm {

template<class T> T from_string(const std::string&);

} // namespace Akm
} // namespace Fred

#endif//FROM_STRING_HH_3AC40C9B47AC09214CE546C9737AEA9D
