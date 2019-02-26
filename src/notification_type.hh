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

enum struct NotificationType
{
    akm_notification_candidate_ko, ///< domain state not ok, domain is not a candidate for AKM
    akm_notification_candidate_ok, ///< domain state seems ok, domain is still a candidate for AKM
    akm_notification_managed_ok, ///< domain switched to AKM
};

std::string to_string(const NotificationType& _notification_type_enum);
int to_db_handle(const NotificationType& _type);

} // namespace Fred::Akm
} // namespace Fred

#endif
