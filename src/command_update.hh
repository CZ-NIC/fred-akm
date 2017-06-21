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
        const unsigned long _maximal_time_between_scan_results,
        const unsigned long _minimal_scan_result_sequence_length_to_update,
        bool _notify_from_last_iteration_only,
        bool _dry_run);


} //namespace Fred::Akm
} //namespace Fred

#endif
