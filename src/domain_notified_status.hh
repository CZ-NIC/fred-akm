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
#ifndef DOMAIN_NOTIFIED_STATUS_HH_BBDA2A17589B41BC810D2B3BC4EF8ABE
#define DOMAIN_NOTIFIED_STATUS_HH_BBDA2A17589B41BC810D2B3BC4EF8ABE

#include "src/domain.hh"
#include "src/domain_status.hh"
#include "src/domain_united_state.hh"
#include "src/enum_conversions.hh"
#include "src/notification_type.hh"
#include "src/scan_result_row.hh"

#include <string>
#include <vector>

namespace Fred {
namespace Akm {


struct DomainNotifiedStatus
{
    DomainNotifiedStatus()
        : domain(),
          serialized_cdnskeys(),
          domain_status(),
          notification_type(),
          last_at()
    {
    }

    /*
    DomainNotifiedStatus(
        const Domain& _domain,
        const std::string& _serialized_cdnskeys, // serialized _cdnskeys
        DomainStatus::DomainStatusType _domain_status,
        NotificationType _notification_type,
        std::string _last_at,
        : domain(_domain),
          serialized_cdnskeys(_serialized_cdnskeys),
          domain_status(_domain_status),
          notification_type(_notification_type),
          last_at(_last_at)
    {
    }
    */

    DomainNotifiedStatus(
            const Domain& _domain,
            const DomainStatus& _domain_status)
        : domain(_domain),
          serialized_cdnskeys(_domain_status.domain_state ? serialize(_domain_status.domain_state->cdnskeys) : ""),
          domain_status(_domain_status.status),
          notification_type(Conversion::Enums::to_notification_type(_domain_status.status)),
          last_at(_domain_status.domain_state ? _domain_status.domain_state->scan_at : _domain_status.scan_iteration.start_at)
    {
    }


    DomainNotifiedStatus(
            const Domain& _domain,
            const DomainUnitedState& _domain_united_state,
            const DomainStatus::DomainStatusType _domain_united_state_status)
        : domain(_domain),
          serialized_cdnskeys(!_domain_united_state.is_empty() ? serialize(_domain_united_state.get_cdnskeys()) : ""),
          domain_status(_domain_united_state_status),
          notification_type(Conversion::Enums::to_notification_type(_domain_united_state_status)),
          //last_at(!_domain_united_state.is_empty() ? _domain_united_state.get_scan_to() : _domain_united_state.scan_iteration.start_at)
          last_at(_domain_united_state.get_scan_to())
    {
    }


    Domain domain;
    std::string serialized_cdnskeys; // serialized cdnskeys
    DomainStatus::DomainStatusType domain_status;
    NotificationType notification_type;
    ScanDateTime last_at;
};

std::string to_string(const DomainNotifiedStatus& _notified_domain_status);

bool are_coherent(const DomainState& _domain_state, const DomainNotifiedStatus& _notified_domain_status);

bool are_coherent(const DomainUnitedState& _domain_united_state, const DomainNotifiedStatus& _notified_domain_status);

} // namespace Fred::Akm
} // namespace Fred

#endif
