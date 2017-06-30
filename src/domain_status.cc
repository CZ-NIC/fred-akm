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

#include "src/domain_status.hh"
#include "src/enum_conversions.hh"

#include <istream>
#include <ostream>
#include <string>

namespace Fred {
namespace Akm {

namespace {

std::string quote(const std::string& str) {
    return "\"" + str + "\"";
}

std::string quote(int value) {
    return std::to_string(value);
}

bool operator==(const DomainStatus& _lhs, const DomainStatus& _rhs)
{
    return
        _lhs.status == _rhs.status &&
        _lhs.scan_iteration == _rhs.scan_iteration &&
        _lhs.domain_state == _rhs.domain_state;
}


bool operator!=(const DomainStatus& _lhs, const DomainStatus& _rhs)
{
    return !(_lhs == _rhs);
}

} // namespace Fred::Akim::{anonymous}


std::ostream& operator<<(std::ostream& os, const DomainStatus& _domain_status)
{
    static const std::string delim = ", ";
    os      << "["
            << quote(_domain_status.status) << delim
            << _domain_status.scan_iteration
            << _domain_status.domain_state.value_or(DomainState())
            << "]";

    return os;
}

// see "src/sqlite/storage.cc"

std::string to_string(const DomainStatus& _domain_status)
{
    static const std::string delim = ", ";
    return std::string("[") +
           quote(to_string(_domain_status.status)) + delim +
           to_string(_domain_status.scan_iteration) +
           to_string(_domain_status.domain_state.value_or(DomainState())) +
           "]";
}

std::string to_string(const DomainStatus::Enum& _domain_status_enum)
{
    switch (_domain_status_enum) {
        case DomainStatus::akm_status_candidate_ok:
                   return "akm_status_candidate_ok";
                   break;
        case DomainStatus::akm_status_candidate_ko:
                   return "akm_status_candidate_ko";
                   break;
        case DomainStatus::akm_status_managed_ok:
                   return "akm_status_managed_ok";
                   break;
    }
}

std::string to_status_string(const DomainStatus& _domain_status)
{
    return _domain_status.status == DomainStatus::akm_status_candidate_ok
            ? "OK"
            : _domain_status.status == DomainStatus::akm_status_candidate_ko
                    ? "KO"
                    : "??";
}

int to_db_handle(const DomainStatus::Enum& _status)
{
    switch (_status) {
        case DomainStatus::akm_status_candidate_ok:
            return 0;
            break;
        case DomainStatus::akm_status_candidate_ko:
            return 1;
            break;
        case DomainStatus::akm_status_managed_ok:
            return 2;
            break;
    }
}

} //namespace Fred::Akm
} //namespace Fred
