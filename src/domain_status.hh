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

#ifndef DOMAIN_STATUS_HH_1404008760D34CDB8E4AFF7A3FA2418F
#define DOMAIN_STATUS_HH_1404008760D34CDB8E4AFF7A3FA2418F

#include "src/domain_state.hh"
#include "src/scan_iteration.hh"

#include <boost/assign/ptr_list_of.hpp>
#include <boost/optional.hpp>

#include <map>
#include <stdexcept>
#include <string>

namespace Fred {
namespace Akm {

struct DomainStatus
{
    enum Enum
    {
        akm_status_candidate_ok, ///< domain state seems ok to switch to AKM
        akm_status_candidate_ko, ///< domain state prevents switching to AKM
        akm_status_managed_ok, ///< domain state prevents switching to AKM
    };

    DomainStatus()
        : status(),
          scan_iteration(),
          domain_state(),
          nameservers()
    {
    }

    DomainStatus(DomainStatus::Enum _status, ScanIteration _scan_iteration, const boost::optional<DomainState>& _domain_state, const std::vector<std::string>& _nameservers)
        : status(_status),
          scan_iteration(_scan_iteration),
          domain_state(_domain_state),
          nameservers(_nameservers)
    {
    }

    Enum status;
    ScanIteration scan_iteration;
    boost::optional<DomainState> domain_state;
    std::vector<std::string> nameservers;
};

std::string to_string(const DomainStatus& _domain_status);
std::string to_string(const DomainStatus::Enum& _domain_status_enum);
int to_db_handle(const DomainStatus::Enum& _status);

} // namespace Fred::Akm
} // namespace Fred

#endif
