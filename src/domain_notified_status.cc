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

#include "src/domain_notified_status.hh"

#include "src/domain.hh"
#include "src/domain_status.hh"
#include "src/enum_conversions.hh"
#include "src/notification_type.hh"
#include "src/scan_date_time.hh"

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


// see "src/sqlite/storage.cc"

std::string to_string(const DomainNotifiedStatus& _domain_notified_status)
{
    static const std::string delim = ", ";
    return  "[" +
            quote(to_string(_domain_notified_status.domain_status)) + delim +
            quote(to_string(_domain_notified_status.last_at)) + delim +
            to_string(_domain_notified_status.domain) + delim +
            quote(_domain_notified_status.serialized_cdnskeys) + delim +
            quote(to_string(_domain_notified_status.notification_type)) +
            "]";
}

bool are_coherent(const DomainState& _domain_state, const DomainNotifiedStatus& _domain_notified_status)
{
    return _domain_state.domain == _domain_notified_status.domain &&
           serialize(_domain_state.cdnskeys) == _domain_notified_status.serialized_cdnskeys;
}

bool are_coherent(const DomainUnitedState& _domain_united_state, const DomainNotifiedStatus& _domain_notified_status)
{
    return _domain_united_state.domain == _domain_notified_status.domain &&
           !_domain_united_state.is_empty() &&
           serialize(_domain_united_state.get_cdnskeys()) == _domain_notified_status.serialized_cdnskeys;
}

} // namespace Fred::Akm
} // namespace Fred
