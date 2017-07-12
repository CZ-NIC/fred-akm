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

#ifndef SCAN_RESULT_HH_598C167E29DF4307BB188D70F565A42A
#define SCAN_RESULT_HH_598C167E29DF4307BB188D70F565A42A

#include "src/cdnskey.hh"
#include "src/domain.hh"

#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {

struct DomainState
{
    DomainState()
        : scan_at(),
          scan_at_seconds(),
          domain(),
          nameserver(),
          nameserver_ip(),
          cdnskeys()
    {
    }

    DomainState(
            const std::string& _scan_at,
            int _scan_at_seconds,
            const Domain& _domain,
            const std::string& _nameserver,
            const std::string& _nameserver_ip,
            const std::map<std::string, Cdnskey>& _cdnskeys)
        : scan_at(_scan_at),
          scan_at_seconds(_scan_at_seconds),
          domain(_domain),
          nameserver(_nameserver),
          nameserver_ip(_nameserver_ip),
          cdnskeys(_cdnskeys)
    {
    }

    DomainState(
            const std::string& _scan_at,
            int _scan_at_seconds,
            const Domain& _domain,
            const std::string& _nameserver,
            const std::string& _nameserver_ip,
            const Cdnskey& _cdnskey)
        : scan_at(_scan_at),
          scan_at_seconds(_scan_at_seconds),
          domain(_domain),
          nameserver(_nameserver),
          nameserver_ip(_nameserver_ip)
    {
        if (_cdnskey.public_key.length()) {
            cdnskeys[to_string(_cdnskey)] = _cdnskey;
        }
    }

    DomainState(const DomainState& _domain_state)
        : scan_at(_domain_state.scan_at),
          scan_at_seconds(_domain_state.scan_at_seconds),
          domain(_domain_state.domain),
          nameserver(_domain_state.nameserver),
          nameserver_ip(_domain_state.nameserver_ip),
          cdnskeys(_domain_state.cdnskeys)
    {
    }

    void add(const Cdnskey& _cdnskey) {
        if (_cdnskey.public_key.length()) {
            cdnskeys[to_string(_cdnskey)] = _cdnskey;
        }
    }

    std::string scan_at;
    int scan_at_seconds;
    Domain domain;
    std::string nameserver;
    std::string nameserver_ip;
    std::map<std::string, Cdnskey> cdnskeys;
};

typedef std::vector<DomainState> DomainStates;

std::string to_string(const DomainState& _domain_state, bool verbose = false);
bool operator==(const DomainState& _lhs, const DomainState& _rhs);

bool has_deletekey(const DomainState& _domain_state);
bool are_coherent(const DomainState& _domain_state, const DomainState& domain_state2);

} // namespace Fred::Akm
} // namespace Fred

#endif
