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

#include "src/domain_state.hh"

#include "src/cdnskey.hh"
#include "src/domain.hh"

#include <algorithm>
#include <istream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


namespace {

std::string quote(const std::string& str) {
    return "\"" + str + "\"";
}

std::string quote(unsigned long long value) {
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}


std::ostream& operator<<(std::ostream& os, const DomainState& domain_state)
{
    static const std::string delim = ", ";
    os << "["
       << quote(domain_state.scan_at) << delim
       << domain_state.domain << delim;
    for (const auto& cdnskey : domain_state.cdnskeys)
    {
        os << cdnskey.second;
    }
    os << "]";
    os << " ("
       << "taken from NS: " << quote(domain_state.nameserver) << delim
       << "with IP: " << quote(domain_state.nameserver_ip) << delim
       << "at: " << quote(domain_state.scan_at)
       << ")";

    return os;
}

// see "src/sqlite/storage.cc"

std::string to_string(const DomainState& domain_state)
{
    static const std::string delim = ", ";
    std::string retval;
    retval = "[" +
            quote(domain_state.scan_at) + delim +
            to_string(domain_state.domain);
            for (const auto& cdnskey : domain_state.cdnskeys)
            {
                retval += delim + to_string(cdnskey.second);
            }
            retval += "]";
            retval += std::string(" (") +
            "taken from NS: " + quote(domain_state.nameserver) + delim +
            "with IP: " + quote(domain_state.nameserver_ip) + delim +
            "at: " + quote(domain_state.scan_at);
    return retval;
}

bool has_deletekey(const DomainState& _domain_state) {
    return std::find_if(
                   _domain_state.cdnskeys.begin(),
                   _domain_state.cdnskeys.end(),
                   [](std::pair<std::string, Cdnskey> cdnskeys_item) {
                     return is_deletekey(cdnskeys_item.second);
                   })
           != _domain_state.cdnskeys.end();
}

bool are_coherent(const DomainState& _first, const DomainState& _second)
{
    if (_first.domain != _second.domain) {
        return false;
    }
    if (_first.cdnskeys != _second.cdnskeys) {
        return false;
    }
    return true;
}

} // namespace Fred::Akm
} // namespace Fred
