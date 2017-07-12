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

#include "scan_iteration.hh"

#include "src/scan_iteration.hh"

#include <istream>
#include <string>

namespace Fred {
namespace Akm {

namespace {

std::string quote(const std::string& str) {
    return "\"" + str + "\"";
}

std::string quote(int value) {
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}


// see "src/sqlite/storage.cc"

std::string to_string(const ScanIteration& _scan_iteration)
{
    static const std::string delim = ", ";
    return std::string("[") +
           quote(_scan_iteration.id) + delim +
           quote(_scan_iteration.start_at) + delim +
           quote(_scan_iteration.start_at_seconds) + delim +
           quote(_scan_iteration.end_at) +
           "]";
}

bool operator==(const ScanIteration& _lhs, const ScanIteration& _rhs)
{
    return
        _lhs.id == _rhs.id &&
        _lhs.start_at == _rhs.start_at &&
        _lhs.start_at_seconds == _rhs.start_at_seconds &&
        _lhs.end_at == _rhs.end_at;
}


bool operator!=(const ScanIteration& _lhs, const ScanIteration& _rhs)
{
    return !(_lhs == _rhs);
}

bool operator<(const ScanIteration& _lhs, const ScanIteration& _rhs)
{
    return _lhs.id < _rhs.id;
}

} //namespace Fred::Akm
} //namespace Fred
