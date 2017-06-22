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

#ifndef DOMAIN_HH_9DD89238F84C428B978A55200B409D98
#define DOMAIN_HH_9DD89238F84C428B978A55200B409D98

#include <string>

namespace Fred {
namespace Akm {


struct Domain
{
    Domain()
        : id(), fqdn(), has_keyset()
    {
    }

    Domain(unsigned long long _id, const std::string& _fqdn, bool _has_keyset)
        : id(_id), fqdn(_fqdn), has_keyset(_has_keyset)
    {
    }

    unsigned long long id;
    std::string fqdn;
    bool has_keyset;
};

bool operator==(const Domain& _lhs, const Domain& _rhs);
bool operator!=(const Domain& _lhs, const Domain& _rhs);
bool operator<(const Domain& _lhs, const Domain& _rhs);

std::ostream& operator<<(std::ostream& os, const Domain& _domain);
std::istream& operator>>(std::istream& is, Domain& _domain);
std::string to_string(const Domain& _domain);

} // namespace Fred
} // namespace Akm

#endif
