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

#ifndef NOTIFICATION_HH_B0C7F73BF9F144C58424D79896AAD48A
#define NOTIFICATION_HH_B0C7F73BF9F144C58424D79896AAD48A

#include "src/i_akm.hh"
#include "src/i_mailer.hh"
#include "src/i_storage.hh"
#include "src/notified_domain_status.hh"

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

void notify_and_save_domain_status(
        const NotifiedDomainStatus& _notified_domain_status,
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const bool _dry_run);


} // namespace Fred::Akm
} // namespace Fred

#endif
