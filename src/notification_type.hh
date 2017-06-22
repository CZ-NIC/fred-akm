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

#ifndef NOTIFICATION_TYPE_HH_1D0A5C1DFBA24C848C92899ED47FBDC8
#define NOTIFICATION_TYPE_HH_1D0A5C1DFBA24C848C92899ED47FBDC8

#include <string>

namespace Fred {
namespace Akm {

struct NotificationType
{
    enum Enum
    {
        akm_notification_candidate_ok, ///< domain state seems ok to switch to AKM
        akm_notification_candidate_ko, ///< domain state prevents switching to AKM
        akm_notification_managed_ok, ///< domain state prevents switching to AKM
    };

    NotificationType()
    {
    }
};

std::string to_string(const NotificationType::Enum& _notification_type_enum);
int to_db_handle(const NotificationType::Enum& _type);

} // namespace Fred::Akm
} // namespace Fred

#endif
