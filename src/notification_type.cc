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

#include "notification_type.hh"

namespace Fred {
namespace Akm {

std::string to_string(const NotificationType::Enum& _notification_type_enum)
{
    switch (_notification_type_enum) {
        case NotificationType::akm_notification_candidate_ok:
                       return "akm_notification_candidate_ok";
                       break;
        case NotificationType::akm_notification_candidate_ko:
                       return "akm_notification_candidate_ko";
                       break;
        case NotificationType::akm_notification_managed_ok:
                       return "akm_notification_managed_ok";
                       break;
    }
}

int to_db_handle(const NotificationType::Enum& _notification_type_enum)
{
    switch (_notification_type_enum) {
        case NotificationType::akm_notification_candidate_ok:
            return 0;
            break;
        case NotificationType::akm_notification_candidate_ko:
            return 1;
            break;
        case NotificationType::akm_notification_managed_ok:
            return 2;
            break;
    }
}

} // namespace Fred::Akm
} // namespace Fred
