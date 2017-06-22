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

#ifndef SCAN_ITERATION_HH_60A1E0DFDE794EB782532E557B3902DE
#define SCAN_ITERATION_HH_60A1E0DFDE794EB782532E557B3902DE

#include <string>

namespace Fred {
namespace Akm {


struct ScanIteration
{
    ScanIteration()
        : id(), start_at(), start_at_seconds(), end_at()
    {
    }

    ScanIteration(int _id, const std::string& _start_at, int start_at_seconds,  const std::string& _end_at)
        : id(_id), start_at(_start_at), start_at_seconds(), end_at(_end_at)
    {
    }

    int id;
    std::string start_at;
    int start_at_seconds;
    std::string end_at;
};

bool operator==(const ScanIteration& _lhs, const ScanIteration& _rhs);
bool operator!=(const ScanIteration& _lhs, const ScanIteration& _rhs);
bool operator<(const ScanIteration& _lhs, const ScanIteration& _rhs);

std::ostream& operator<<(std::ostream& os, const ScanIteration& _scan_iteration);
std::istream& operator>>(std::istream& is, ScanIteration& _scan_iteration);
std::string to_string(const ScanIteration& _scan_iteration);

} // namespace Fred
} // namespace Akm

#endif
