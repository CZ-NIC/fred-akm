/*
 * Copyright (C) 2017  CZ.NIC, z. s. p. o.
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
#include "src/notification_type.hh"

#include <stdexcept>

namespace Fred {
namespace Akm {

std::string to_string(const NotificationType& _notification_type)
{
    switch (_notification_type)
    {
        case NotificationType::akm_notification_candidate_ok:
                       return "akm_notification_candidate_ok";
        case NotificationType::akm_notification_candidate_ko:
                       return "akm_notification_candidate_ko";
        case NotificationType::akm_notification_managed_ok:
                       return "akm_notification_managed_ok";
    }
    throw std::invalid_argument("Fred::Akm::NotificationType value out of range");
}

int to_db_handle(const NotificationType& _notification_type)
{
    switch (_notification_type)
    {
        case NotificationType::akm_notification_candidate_ok:
            return 0;
        case NotificationType::akm_notification_candidate_ko:
            return 1;
        case NotificationType::akm_notification_managed_ok:
            return 2;
    }
    throw std::invalid_argument("Fred::Akm::NotificationType value out of range");
}

} // namespace Fred::Akm
} // namespace Fred
