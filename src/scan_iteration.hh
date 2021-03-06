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
#ifndef SCAN_ITERATION_HH_60A1E0DFDE794EB782532E557B3902DE
#define SCAN_ITERATION_HH_60A1E0DFDE794EB782532E557B3902DE

#include "src/scan_date_time.hh"

#include <string>

namespace Fred {
namespace Akm {


struct ScanIteration
{
    ScanIteration()
        : id(),
          start_at(),
          end_at()
    {
    }

    ScanIteration(
            int _id,
            const ScanDateTime& _start_at,
            const ScanDateTime& _end_at)
        : id(_id),
          start_at(_start_at),
          end_at(_end_at)
    {
    }

    int id;
    ScanDateTime start_at;
    ScanDateTime end_at;
};

bool operator==(const ScanIteration& _lhs, const ScanIteration& _rhs);
bool operator!=(const ScanIteration& _lhs, const ScanIteration& _rhs);
bool operator<(const ScanIteration& _lhs, const ScanIteration& _rhs);

std::string to_string(const ScanIteration& _scan_iteration);

} // namespace Fred
} // namespace Akm

#endif
