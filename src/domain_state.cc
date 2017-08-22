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
#include "src/scan_date_time.hh"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


namespace {

std::string quote(const std::string& str)
{
    return "\"" + str + "\"";
}

std::string quote(unsigned long long value)
{
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}


// see "src/sqlite/storage.cc"

std::string to_string(const DomainState& _domain_state, const bool verbose)
{
    static const std::string delim = ", ";
    std::string retval;
    retval = "[" +
            quote(_domain_state.scan_at.scan_date_time) + delim +
            to_string(_domain_state.domain);
    for (const auto& cdnskey : _domain_state.cdnskeys)
    {
        retval += delim + to_string(cdnskey.second);
    }
    retval += "]";
    if (verbose)
    {
        retval += std::string(" (") +
                  "taken from NS: " + quote(_domain_state.nameserver) + delim +
                  "with IP: " + quote(_domain_state.nameserver_ip) + delim +
                  "at: " + quote(to_string(_domain_state.scan_at));
    }
    return retval;
}

bool operator==(const DomainState& _lhs, const DomainState& _rhs)
{
    return
        _lhs.scan_at == _rhs.scan_at &&
        _lhs.domain == _rhs.domain &&
        _lhs.nameserver == _rhs.nameserver &&
        _lhs.nameserver_ip == _rhs.nameserver_ip &&
        _lhs.cdnskeys == _rhs.cdnskeys;
}

bool operator!=(const DomainState& _lhs, const DomainState& _rhs)
{
    return !(_lhs == _rhs);
}

bool are_coherent(const DomainState& _first_domain_state, const DomainState& _second_domain_state)
{
    return _first_domain_state.domain == _second_domain_state.domain &&
           _first_domain_state.cdnskeys == _second_domain_state.cdnskeys;
}

} // namespace Fred::Akm
} // namespace Fred
