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

#include "src/domain_united_state.hh"

#include <boost/optional.hpp>

#include <algorithm>

namespace Fred {
namespace Akm {

namespace {

std::string quote(const std::string& str)
{
    return "\"" + str + "\"";
}

} // namespace Fred::Akim::{anonymous}


std::string to_string(const DomainUnitedState& _domain_united_state, const bool verbose)
{
    static const std::string delim = ", ";
    std::string retval;
    retval = "[" +
            quote(!_domain_united_state.is_empty() ? to_string(_domain_united_state.get_scan_from()) : "") + delim +
            quote(!_domain_united_state.is_empty() ? to_string(_domain_united_state.get_scan_to()) : "") + delim +
            to_string(_domain_united_state.domain);
    if (_domain_united_state.is_coherent())
    {
        for (const auto& cdnskey : _domain_united_state.get_cdnskeys())
        {
            retval += delim + to_string(cdnskey.second);
        }
    }
    else
    {
        retval += delim + "INVALID";
    }
    retval += "]";
    //if (verbose)
    //{
    //    retval += std::string(" (") +
    //              "taken from NS: " + quote(_domain_united_state.nameserver) + delim +
    //              "with IP: " + quote(_domain_united_state.nameserver_ip) + delim +
    //              "at: " + quote(_domain_united_state.scan_at);
    //}
    return retval;
}

bool is_narrow(const DomainUnitedState& _domain_united_state, int seconds_max)
{
    return !_domain_united_state.is_empty() &&
            _domain_united_state.get_scan_to().scan_seconds -
            _domain_united_state.get_scan_from().scan_seconds <=
            seconds_max;
}

bool is_recent(const DomainUnitedState& _domain_united_state, int seconds_max, int current_unix_time)
{
    return !_domain_united_state.is_empty() &&
            current_unix_time -
            _domain_united_state.get_scan_from().scan_seconds <=
            seconds_max;
}

bool are_close(
        const DomainUnitedState& _first_domain_united_state,
        const DomainUnitedState& _second_domain_united_state,
        int seconds_max)
{
    return !_first_domain_united_state.is_empty() &&
           !_second_domain_united_state.is_empty() &&
            std::max(_first_domain_united_state.get_scan_to().scan_seconds,
                     _second_domain_united_state.get_scan_to().scan_seconds) -
            std::min(_first_domain_united_state.get_scan_from().scan_seconds,
                     _second_domain_united_state.get_scan_from().scan_seconds)
            <= seconds_max;
}

bool are_coherent(
        const DomainUnitedState& _first_domain_united_state,
        const DomainUnitedState& _second_domain_united_state)
{
    return _first_domain_united_state.is_coherent() &&
           _second_domain_united_state.is_coherent() &&
           _first_domain_united_state.domain == _second_domain_united_state.domain &&
           _first_domain_united_state.get_cdnskeys() == _second_domain_united_state.get_cdnskeys();
}

bool is_dnssec_turn_off_requested(const DomainUnitedState& _domain_united_state)
{
    return !_domain_united_state.is_empty() &&
           _domain_united_state.get_cdnskeys().size() == 1 &&
           is_deletekey((*_domain_united_state.get_cdnskeys().cbegin()).second);
}

} // namespace Fred::Akm
} // namespace Fred
