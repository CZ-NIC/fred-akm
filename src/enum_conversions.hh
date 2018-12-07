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
#ifndef ENUM_CONVERSIONS_HH_B9B0AF5BD3B9445C863F18AE3084992E
#define ENUM_CONVERSIONS_HH_B9B0AF5BD3B9445C863F18AE3084992E

#include "src/domain_status.hh"
#include "src/notification_type.hh"

namespace Conversion {
namespace Enums {

inline Fred::Akm::NotificationType to_notification_type(Fred::Akm::DomainStatus::DomainStatusType value)
{
    switch (value)
    {
        case Fred::Akm::DomainStatus::DomainStatusType::akm_status_candidate_ok:
            return Fred::Akm::NotificationType::akm_notification_candidate_ok;

        case Fred::Akm::DomainStatus::DomainStatusType::akm_status_candidate_ko:
            return Fred::Akm::NotificationType::akm_notification_candidate_ko;

        case Fred::Akm::DomainStatus::DomainStatusType::akm_status_managed_ok:
            return Fred::Akm::NotificationType::akm_notification_managed_ok;
    }
    throw std::invalid_argument("value doesn't exist in Fred::Akm::DomainStatus::DomainStatusType");
}

inline std::string to_template_name(Fred::Akm::NotificationType value)
{
    switch (value)
    {
        case Fred::Akm::NotificationType::akm_notification_candidate_ok:
            return "akm_candidate_state_ok";

        case Fred::Akm::NotificationType::akm_notification_candidate_ko:
            return "akm_candidate_state_ko";

        case Fred::Akm::NotificationType::akm_notification_managed_ok:
            return "akm_keyset_update";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Akm::NotificationType");
}

template <typename T>
inline T from_db_handle(int _db_handle);

template <>
inline Fred::Akm::NotificationType from_db_handle<Fred::Akm::NotificationType>(int _type)
{
    switch (_type) {
        case 0:
            return Fred::Akm::NotificationType::akm_notification_candidate_ok;
            break;
        case 1:
            return Fred::Akm::NotificationType::akm_notification_candidate_ko;
            break;
        case 2:
            return Fred::Akm::NotificationType::akm_notification_managed_ok;
            break;
    }
    throw std::invalid_argument("cannot convert value to Fred::Akm::NotificationType");
}

template <>
inline Fred::Akm::DomainStatus::DomainStatusType from_db_handle<Fred::Akm::DomainStatus::DomainStatusType>(int _type)
{
    switch (_type) {
        case 0:
            return Fred::Akm::DomainStatus::DomainStatusType::akm_status_candidate_ok;
            break;
        case 1:
            return Fred::Akm::DomainStatus::DomainStatusType::akm_status_candidate_ko;
            break;
        case 2:
            return Fred::Akm::DomainStatus::DomainStatusType::akm_status_managed_ok;
            break;
    }
    throw std::invalid_argument("cannot convert value to Fred::Akm::DomanStatus::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif
