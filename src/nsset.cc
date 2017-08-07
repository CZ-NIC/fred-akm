/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/nsset.hh"

#include <string>

namespace Fred {
namespace Akm {

namespace {

std::string quote(const std::string& str)
{
    return "\"" + str + "\"";
}

std::string quote(int value)
{
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}

std::string to_string(const Nsset& _nsset)
{
    static const std::string delim = ", ";
    std::string retval;
    retval = "[";
    for (const auto& nameserver : _nsset.nameservers)
    {
        retval += quote(nameserver);
        if (&nameserver != &_nsset.nameservers.back())
        {
            retval += delim;
        }
    }
    retval += "]";
    return retval;
}

} // namespace Fred::Akm
} // namespace Fred
