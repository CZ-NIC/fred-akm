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
#ifndef COMMAND_UPDATE_HH_756843D0C1F14AE28487B8CC7820AD57
#define COMMAND_UPDATE_HH_756843D0C1F14AE28487B8CC7820AD57

#include <string>

#include "src/i_akm.hh"
#include "src/i_mailer.hh"
#include "src/i_storage.hh"

namespace Fred {
namespace Akm {


void command_update(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        unsigned long long _maximal_time_between_scan_results,
        unsigned long long _minimal_scan_result_sequence_length_to_update,
        bool _align_to_start_of_day,
        bool _dry_run,
        bool _fake_contact_emails);


} //namespace Fred::Akm
} //namespace Fred

#endif
