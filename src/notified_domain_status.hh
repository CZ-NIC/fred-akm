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

#ifndef NOTIFIED_DOMAIN_STATUS_HH_BBDA2A17589B41BC810D2B3BC4EF8ABE
#define NOTIFIED_DOMAIN_STATUS_HH_BBDA2A17589B41BC810D2B3BC4EF8ABE

#include "src/domain.hh"
#include "src/domain_status.hh"
#include "src/enum_conversions.hh"
#include "src/notification_type.hh"
#include "src/scan_result_row.hh"

#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


struct NotifiedDomainStatus
{
    NotifiedDomainStatus()
        : domain(),
          serialized_cdnskeys(),
          domain_status(),
          notification_type(),
          last_at(),
          last_at_seconds()
    {
    }

    /*
    NotifiedDomainStatus(
        const Domain& _domain,
        const std::string& _serialized_cdnskeys, // serialized _cdnskeys
        DomainStatus::Enum _domain_status,
        NotificationType::Enum _notification_type,
        std::string _last_at,
        int _last_at_seconds)
        : domain(_domain),
          serialized_cdnskeys(_serialized_cdnskeys),
          domain_status(_domain_status),
          notification_type(_notification_type),
          last_at(_last_at),
          last_at_seconds(_last_at_seconds)
    {
    }
    */

    NotifiedDomainStatus(
            const Domain& _domain,
            const DomainStatus& _domain_status)
        : domain(_domain),
          serialized_cdnskeys(_domain_status.domain_state ? serialize(_domain_status.domain_state->cdnskeys) : ""),
          domain_status(_domain_status.status),
          notification_type(Conversion::Enums::to_notification_type(_domain_status.status)),
          last_at(_domain_status.domain_state ? _domain_status.domain_state->scan_at : _domain_status.scan_iteration.start_at),
          last_at_seconds(_domain_status.domain_state ? _domain_status.domain_state->scan_at_seconds : _domain_status.scan_iteration.start_at_seconds)
    {
    }


    Domain domain;
    std::string serialized_cdnskeys; // serialized cdnskeys
    DomainStatus::Enum domain_status;
    NotificationType::Enum notification_type;
    std::string last_at;
    int last_at_seconds;
};

std::ostream& operator<<(std::ostream& os, const NotifiedDomainStatus& _notified_domain_status);
std::string to_string(const NotifiedDomainStatus& _notified_domain_status);
std::string to_status_string(const NotifiedDomainStatus& _notified_domain_status);
bool are_coherent(const DomainState& _domain_state, const NotifiedDomainStatus& _notified_domain_status);

} // namespace Fred::Akm
} // namespace Fred

#endif
