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
        : domain_id(),
          domain_name(),
          has_keyset(),
          serialized_cdnskeys(),
          notification_type(),
          last_at()
    {
    }

    NotifiedDomainState(
        unsigned long long _domain_id,
        const std::string& _domain_name,
        bool _has_keyset,
        const std::string& _serialized_cdnskeys, // serialized _cdnskeys
        int _notification_type,
        std::string _last_at)
        : domain_id(_domain_id),
          domain_name(_domain_name),
          has_keyset(_has_keyset),
          serialized_cdnskeys(_serialized_cdnskeys),
          notification_type(_notification_type),
          last_at(_last_at)
    {
    }

    unsigned long long domain_id;
    std::string domain_name;
    bool has_keyset;
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
