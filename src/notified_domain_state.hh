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

#ifndef NOTIFIED_DOMAIN_STATE_HH_BBDA2A17589B41BC810D2B3BC4EF8ABE
#define NOTIFIED_DOMAIN_STATE_HH_BBDA2A17589B41BC810D2B3BC4EF8ABE

#include "src/domain.hh"
#include "src/scan_result_row.hh"

#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


struct NotifiedDomainState
{
    NotifiedDomainState()
        : domain(),
          serialized_cdnskeys(),
          notification_type(),
          last_at()
    {
    }

    NotifiedDomainState(
        const Domain& _domain,
        const std::string& _serialized_cdnskeys, // serialized _cdnskeys
        int _notification_type,
        std::string _last_at)
        : domain(_domain),
          serialized_cdnskeys(_serialized_cdnskeys),
          notification_type(_notification_type),
          last_at(_last_at)
    {
    }

    Domain domain;
    std::string serialized_cdnskeys; // serialized cdnskeys
    int notification_type;
    std::string last_at;
};

std::ostream& operator<<(std::ostream& os, const NotifiedDomainState& notified_domain_state);
std::istream& operator>>(std::istream& is, NotifiedDomainState& notified_domain_state);
std::string to_string(const NotifiedDomainState& notified_domain_state);

} // namespace Fred::Akm
} // namespace Fred

#endif
