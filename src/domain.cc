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
#include "src/domain.hh"

#include <ostream>
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

std::string to_string(const Domain& _domain)
{
    static const std::string delim = ", ";
    return std::string("[") +
           quote(_domain.id) + delim +
           quote(_domain.fqdn) + delim +
           quote(to_string(_domain.scan_type)) +
           "]";
}

bool operator==(const Domain& _lhs, const Domain& _rhs)
{
    return
        _lhs.id == _rhs.id &&
        _lhs.fqdn == _rhs.fqdn &&
        _lhs.scan_type == _rhs.scan_type;
}


bool operator!=(const Domain& _lhs, const Domain& _rhs)
{
    return !(_lhs == _rhs);
}

bool operator<(const Domain& _lhs, const Domain& _rhs)
{
    if (_lhs.fqdn != _rhs.fqdn)
    {
        return _lhs.fqdn < _rhs.fqdn;
    }
    if (_lhs.id != _rhs.id)
    {
        return _lhs.id < _rhs.id;
    }
    return _lhs.scan_type < _rhs.scan_type;
}

bool DomainLexicographicalComparator::operator()(const Domain& _lhs, const Domain& _rhs) const
{
    if (_lhs.fqdn != _rhs.fqdn)
    {
        return _lhs.fqdn < _rhs.fqdn;
    }
    return _lhs.id < _rhs.id;
}

} // namespace Fred::Akm
} // namespace Fred
