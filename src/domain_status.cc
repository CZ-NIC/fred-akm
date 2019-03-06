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
#include "src/domain_status.hh"
#include "src/enum_conversions.hh"

#include <istream>
#include <ostream>
#include <string>

namespace Fred {
namespace Akm {

namespace {

std::string quote(const std::string& str)
{
    return "\"" + str + "\"";
}

} // namespace Fred::Akim::{anonymous}


// see "src/sqlite/storage.cc"

std::string to_string(const DomainStatus& _domain_status)
{
    static const std::string delim = ", ";
    return std::string("[") +
           quote(to_string(_domain_status.status)) + delim +
           to_string(_domain_status.domain_state ? *_domain_status.domain_state : DomainState()) +
           to_string(_domain_status.scan_iteration) +
           "]";
}

std::string to_string(const DomainStatus::DomainStatusType& _domain_status_type)
{
    switch (_domain_status_type)
    {
        case DomainStatus::DomainStatusType::akm_status_candidate_ok:
                   return "akm_status_candidate_ok";
                   break;
        case DomainStatus::DomainStatusType::akm_status_candidate_ko:
                   return "akm_status_candidate_ko";
                   break;
        case DomainStatus::DomainStatusType::akm_status_managed_ok:
                   return "akm_status_managed_ok";
                   break;
    }
    throw std::invalid_argument("Fred::Akm::DomainStatus::DomainStatusType value out of range");
}

int to_db_handle(const DomainStatus::DomainStatusType& _domain_status_type)
{
    switch (_domain_status_type)
    {
        case DomainStatus::DomainStatusType::akm_status_candidate_ok:
            return 0;
            break;
        case DomainStatus::DomainStatusType::akm_status_candidate_ko:
            return 1;
            break;
        case DomainStatus::DomainStatusType::akm_status_managed_ok:
            return 2;
            break;
    }
    throw std::invalid_argument("Fred::Akm::DomainStatus::DomainStatusType value out of range");
}

} //namespace Fred::Akm
} //namespace Fred
