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
#ifndef DOMAIN_HH_9DD89238F84C428B978A55200B409D98
#define DOMAIN_HH_9DD89238F84C428B978A55200B409D98

#include "src/scan_type.hh"

#include <string>

namespace Fred {
namespace Akm {


struct Domain
{
    Domain()
        : id(), fqdn(), scan_type()
    {
    }

    Domain(unsigned long long _id, const std::string& _fqdn, ScanType _scan_type)
        : id(_id), fqdn(_fqdn), scan_type(_scan_type)
    {
    }

    unsigned long long id;
    std::string fqdn;
    ScanType scan_type;
};

bool operator==(const Domain& _lhs, const Domain& _rhs);
bool operator!=(const Domain& _lhs, const Domain& _rhs);
bool operator<(const Domain& _lhs, const Domain& _rhs);

struct DomainLexicographicalComparator
{
    bool operator()(const Domain& _lhs, const Domain& _rhs) const;
};

std::string to_string(const Domain& _domain);


struct DomainHash
{
    size_t operator()(const Domain& _domain) const
    {
        return std::hash<std::string>()(_domain.fqdn)
            ^ std::hash<unsigned long long>()(_domain.id)
            ^ std::hash<std::string>()(to_string(_domain.scan_type));
    }
};


struct DomainEqual
{
    bool operator()(const Domain& _left, const Domain& _right) const
    {
        return _left.id == _right.id
            && _left.fqdn == _right.fqdn
            && _left.scan_type == _right.scan_type;
    }
};


} // namespace Fred
} // namespace Akm

#endif
