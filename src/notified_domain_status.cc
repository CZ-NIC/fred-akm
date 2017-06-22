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

#include "src/notified_domain_status.hh"

#include "src/domain.hh"
#include "src/notification_type.hh"
#include "src/domain_status.hh"
#include "src/enum_conversions.hh"

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


std::ostream& operator<<(std::ostream& os, const NotifiedDomainStatus& _notified_domain_status)
{
    static const std::string delim = ", ";
    os      << "["
            << _notified_domain_status.domain << delim
            << quote(_notified_domain_status.serialized_cdnskeys) << delim
            << quote(to_string(_notified_domain_status.domain_status)) << delim
            << quote(to_string(_notified_domain_status.notification_type)) << delim
            << quote(_notified_domain_status.last_at)
            << quote(_notified_domain_status.last_at_seconds)
            << "]";

    return os;
}

// see "src/sqlite/storage.cc"

std::string to_string(const NotifiedDomainStatus& _notified_domain_status)
{
    static const std::string delim = ", ";
    return  "[" +
            to_string(_notified_domain_status.domain) + delim +
            quote(_notified_domain_status.serialized_cdnskeys) + delim +
            quote(to_string(_notified_domain_status.domain_status)) + delim +
            quote(to_string(_notified_domain_status.notification_type)) + delim +
            quote(_notified_domain_status.last_at) + delim +
            quote(_notified_domain_status.last_at_seconds) +
            "]";
}

std::string to_status_string(const NotifiedDomainStatus& _notified_domain_status)
{

    return _notified_domain_status.notification_type ==
           Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ok)
                   ? "OK"
                   : _notified_domain_status.notification_type ==
                     Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ko)
                             ? "KO"
                             : "??";
}

bool are_coherent(const DomainState& _domain_state, const NotifiedDomainStatus& _notified_domain_status)
{
    return
            _domain_state.domain == _notified_domain_status.domain &&
            serialize(_domain_state.cdnskeys) == _notified_domain_status.serialized_cdnskeys;
}

} // namespace Fred::Akm
} // namespace Fred
