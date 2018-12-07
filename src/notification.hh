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
#ifndef NOTIFICATION_HH_B0C7F73BF9F144C58424D79896AAD48A
#define NOTIFICATION_HH_B0C7F73BF9F144C58424D79896AAD48A

#include "src/i_akm.hh"
#include "src/i_mailer.hh"
#include "src/i_storage.hh"
#include "src/domain_notified_status.hh"

#include <stdexcept>

namespace Fred {
namespace Akm {

struct NotificationFailed
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "notification failed";
    }
};

struct NotificationNotPossible
    : virtual std::exception
{
    virtual const char* what() const throw ()
    {
        return "notification not possible";
    }
};

void save_domain_status(
        const DomainNotifiedStatus& _domain_notified_status,
        const IStorage& _storage,
        const bool _dry_run);

void notify_and_save_domain_status(
        const DomainNotifiedStatus& _domain_notified_status,
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const bool _dry_run,
        const bool _fake_contact_emails);


} // namespace Fred::Akm
} // namespace Fred

#endif
