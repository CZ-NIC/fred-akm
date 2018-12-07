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
#include "src/keyset.hh"
#include "src/dnskey.hh"

#include <boost/algorithm/string/join.hpp>

#include <ostream>
#include <string>

namespace Fred {
namespace Akm {

std::string to_string(const Keyset& _keyset)
{
    static const std::string delim = ", ";
    std::string retval;
    retval = "[";
    for (const auto& dnskey : _keyset.dnskeys)
    {
        retval += to_string(dnskey);
        if (&dnskey != &_keyset.dnskeys.back())
        {
            retval += delim;
        }
    }
    retval += "]";
    return retval;
}

} // namespace Fred::Akm
} // namespace Fred
