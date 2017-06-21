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

#include "src/cdnskey.hh"

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


std::ostream& operator<<(std::ostream& os, const Cdnskey& _cdnskey)
{
    static const std::string delim = ", ";
    os      << "["
            << quote(_cdnskey.status) << delim
            << quote(_cdnskey.flags) << delim
            << quote(_cdnskey.proto) << delim
            << quote(_cdnskey.alg) << delim
            << quote(_cdnskey.public_key)
            << "]";

    return os;
}

// see "src/sqlite/storage.cc"
std::istream& operator>>(std::istream& is, Cdnskey& _cdnskey)
{
    try {
        is
                >> _cdnskey.status
                >> _cdnskey.flags
                >> _cdnskey.proto
                >> _cdnskey.alg
                >> _cdnskey.public_key;
    }
    catch (...)
    {
        is.setstate(std::ios::failbit);
    }
    return is;
}

std::string to_string(const Cdnskey& _cdnskey)
{
    static const std::string delim = ", ";
    return std::string("[") +
           //quote(_cdnskey.status) + delim +
           "flags: " + quote(_cdnskey.flags) + delim +
           "protocol: " + quote(_cdnskey.proto) + delim +
           "algorithm: " + quote(_cdnskey.alg) + delim +
           "key: " + quote(_cdnskey.public_key) +
           "]";
}


bool operator==(const Cdnskey& _lhs, const Cdnskey& _rhs)
{
    return
        _lhs.status == _rhs.status &&
        _lhs.flags == _rhs.flags &&
        _lhs.proto == _rhs.proto &&
        _lhs.alg == _rhs.alg &&
        _lhs.public_key == _rhs.public_key;
}


bool operator!=(const Cdnskey& _lhs, const Cdnskey& _rhs)
{
    return !(_lhs == _rhs);
}

std::string serialize(const std::map<std::string, Cdnskey>& _cdnskeys)
{
    //boost::algorithm::join(neweset_domain_state->_cdnskeys | boost::adaptors::map_values, ","),
    const std::string delim = "|";
    const std::string result =
            std::accumulate(
                    _cdnskeys.begin(),
                    _cdnskeys.end(),
                    std::string(),
                    [delim](const std::string& s, const std::pair<const std::string, const Cdnskey&>& p) {
                        return s + (s.empty() ? std::string() : delim) + to_string(p.second);
                    });
    return result;
}

bool is_valid(const Cdnskey& _cdnskey)
{
    if (_cdnskey.proto != 3) {
        return false;
    }
    if (_cdnskey.public_key.empty()) {
        return false;
    }
    return true;
}

bool is_empty(const Cdnskey& _cdnskey)
{
    return _cdnskey.public_key.empty();
}

// RFC 8078 section 4. DNSSEC Delete Algorithm
bool is_deletekey(const Cdnskey& _cdnskey)
{
    const std::string base64_zero = "AA==";
    if (_cdnskey.flags == 0 &&
        _cdnskey.proto == 3 &&
        _cdnskey.alg == 0 &&
        _cdnskey.public_key == base64_zero)
    {
        return true;
    }
    return false;
}

} //namespace Fred::Akm
} //namespace Fred
