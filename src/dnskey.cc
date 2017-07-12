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

#include "src/dnskey.hh"

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

#include <istream>
#include <ostream>
#include <string>
#include <vector>

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

std::string to_string(const Dnskey& _dnskey)
{
    static const std::string delim = ", ";
    return std::string("[") +
           "flags: " + quote(_dnskey.flags) + delim +
           "protocol: " + quote(_dnskey.proto) + delim +
           "algorithm: " + quote(_dnskey.alg) + delim +
           "key: " + quote(_dnskey.public_key) +
           "]";
}


bool operator==(const Dnskey& _lhs, const Dnskey& _rhs)
{
    return
        _lhs.flags == _rhs.flags &&
        _lhs.proto == _rhs.proto &&
        _lhs.alg == _rhs.alg &&
        _lhs.public_key == _rhs.public_key;
}


bool operator!=(const Dnskey& _lhs, const Dnskey& _rhs)
{
    return !(_lhs == _rhs);
}

} //namespace Fred::Akm
} //namespace Fred
