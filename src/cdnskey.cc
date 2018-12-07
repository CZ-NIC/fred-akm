/*
 * Copyright (C) 2017  CZ.NIC, z. s. p. o.
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
#include "src/cdnskey.hh"

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

#include <string>
#include <vector>
#include <numeric>

namespace Fred {
namespace Akm {

namespace {

std::string quote(const std::string& str)
{
    return "\"" + str + "\"";
}

std::string quote(int value)
{
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}


// see "src/sqlite/storage.cc"

// do not change this, used for serialization
std::string to_string(const Cdnskey& _cdnskey)
{
    static const std::string delim = ", ";
    return std::string("[") +
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
                    [delim](const std::string& s, const std::pair<const std::string, const Cdnskey&>& p)
                    {
                        return s + (s.empty() ? std::string() : delim) + to_string(p.second);
                    });
    return result;
}

bool is_valid(const Cdnskey& _cdnskey)
{
    // we accept everything
    return true;
}

bool is_empty(const Cdnskey& _cdnskey)
{
    return _cdnskey.public_key.empty();
}

// RFC 8078 section 4. DNSSEC Delete Algorithm
bool is_deletekey(const Cdnskey& _cdnskey)
{
    // if (_cdnskey.alg == 0)
    // {
    //     return true;
    // }
    const std::string base64_encoded_zero = "AA==";
    if (_cdnskey.flags == 0 &&
        _cdnskey.proto == 3 &&
        _cdnskey.alg == 0 &&
        _cdnskey.public_key == base64_encoded_zero)
    {
        return true;
    }
    return false;
}

} //namespace Fred::Akm
} //namespace Fred
