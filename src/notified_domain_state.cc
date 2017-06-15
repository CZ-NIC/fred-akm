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

#include "src/notified_domain_state.hh"

#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


namespace {

std::string quote(const std::string& str) {
    return "\"" + str + "\"";
}

// int, bool, ...
std::string quote(int value) {
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}


std::ostream& operator<<(std::ostream& os, const NotifiedDomainState& notified_domain_state)
{
    static const std::string delim = ", ";
    os      << "["
            << quote(notified_domain_state.domain_id) << delim
            << quote(notified_domain_state.domain_name) << delim
            << quote(notified_domain_state.has_keyset) << delim
            << quote(notified_domain_state.serialized_cdnskeys) << delim
            << quote(notified_domain_state.notification_type) << delim
            << quote(notified_domain_state.last_at)
            << "]";

    return os;
}

// see "src/sqlite/storage.cc"
std::istream& operator>>(std::istream& is, NotifiedDomainState& notified_domain_state)
{
    try {
        is
                >> notified_domain_state.domain_id
                >> notified_domain_state.domain_name
                >> notified_domain_state.has_keyset
                >> notified_domain_state.serialized_cdnskeys
                >> notified_domain_state.notification_type
                >> notified_domain_state.last_at;
    }
    catch (...)
    {
        is.setstate(std::ios::failbit);
    }
    return is;
}

std::string to_string(const NotifiedDomainState& notified_domain_state)
{
    static const std::string delim = ", ";
    return  "[" +
            quote(notified_domain_state.domain_id) + delim +
            quote(notified_domain_state.domain_name) + delim +
            quote(notified_domain_state.has_keyset) + delim +
            quote(notified_domain_state.serialized_cdnskeys) + delim +
            quote(notified_domain_state.notification_type) + delim +
            quote(notified_domain_state.last_at) +
            "]";
}


} // namespace Fred::Akm
} // namespace Fred
