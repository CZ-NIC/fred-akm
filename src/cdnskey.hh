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

#ifndef CDNSKEY_HH_F181841E702D475792361A35B7A089FC
#define CDNSKEY_HH_F181841E702D475792361A35B7A089FC

#include <map>
#include <string>

namespace Fred {
namespace Akm {


struct Cdnskey
{
    Cdnskey()
        : status(),
          flags(),
          proto(),
          alg(),
          public_key()
    {
    }

    Cdnskey(
            const std::string & _status,
            int _flags,
            int _proto,
            int _alg,
            const std::string & _public_key)
        : status(_status),
          flags(_flags),
          proto(_proto),
          alg(_alg),
          public_key(_public_key)
    {
    }

    std::string status;
    int flags;
    int proto;
    int alg;
    std::string public_key;
};

bool operator==(const Cdnskey& _lhs, const Cdnskey& _rhs);
bool operator!=(const Cdnskey& _lhs, const Cdnskey& _rhs);

std::ostream& operator<<(std::ostream& os, const Cdnskey& _cdnskey);
std::istream& operator>>(std::istream& is, Cdnskey& _cdnskey);
std::string to_string(const Cdnskey& _cdnskey);

std::string serialize(const std::map<std::string, Cdnskey>& _cdnskeys);
bool is_valid(const Cdnskey& _cdnskey);
bool is_empty(const Cdnskey& _cdnskey);
bool is_deletekey(const Cdnskey& _cdnskey);

} // namespace Fred::Akm
} // namespace Fred

#endif
