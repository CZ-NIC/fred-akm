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

#ifndef COMMAND_CLEAN_HH_68F8F773D4E2484480F97BE6CDD65A71
#define COMMAND_CLEAN_HH_68F8F773D4E2484480F97BE6CDD65A71

#include <string>

#include "src/i_storage.hh"

namespace Fred {
namespace Akm {


void command_clean(
        const IStorage& _storage,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        bool _align_to_start_of_day);


} //namespace Fred::Akm
} //namespace Fred

#endif
