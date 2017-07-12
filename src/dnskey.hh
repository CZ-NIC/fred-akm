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

#ifndef DNSKEY_HH_CD011C58EA034759988E61E7CF1AAB2B
#define DNSKEY_HH_CD011C58EA034759988E61E7CF1AAB2B

#include <map>
#include <string>

namespace Fred {
namespace Akm {


struct Dnskey
{
    Dnskey()
        : flags(),
          proto(),
          alg(),
          public_key()
    {
    }

    Dnskey(
            int _flags,
            int _proto,
            int _alg,
            const std::string & _public_key)
        : flags(_flags),
          proto(_proto),
          alg(_alg),
          public_key(_public_key)
    {
    }

    int flags;
    int proto;
    int alg;
    std::string public_key;
};

bool operator==(const Dnskey& _lhs, const Dnskey& _rhs);
bool operator!=(const Dnskey& _lhs, const Dnskey& _rhs);

std::string to_string(const Dnskey& _dnskey);

} // namespace Fred::Akm
} // namespace Fred

#endif
