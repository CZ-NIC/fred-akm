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

#ifndef DOMAIN_STATE_STACK_HH_13945F8D78934FF6A7CFE9B6A6CBAC06
#define DOMAIN_STATE_STACK_HH_13945F8D78934FF6A7CFE9B6A6CBAC06

#include "src/cdnskey.hh"
#include "src/nameserver_domains.hh"
#include "src/scan_result_row.hh"
#include "src/domain_state.hh"
#include "src/utils.hh"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>

namespace Fred {
namespace Akm {

struct DomainStateStack {
    typedef std::vector<DomainState> DomainStates;
    typedef std::string Nameserver;
    typedef std::string NameserverIp;
    typedef std::map<NameserverIp, DomainStates> NameserverIps;
    typedef std::map<Nameserver, NameserverIps> Nameservers;
    typedef std::map<Domain, Nameservers> Domains;

    DomainStateStack(const ScanResultRows& _scan_result_rows);

    Domains domains;
};

boost::optional<DomainState> get_last_domain_state(
        const Domain& _domain,
        const DomainStateStack::Nameservers& _nameservers,
        int _scan_result_row_timediff_max,
        int _scan_result_row_sequence_timediff_min,
        bool& _domain_nameservers_coherent);

void print(const DomainStateStack& _haystack);

bool operator<(const Domain& lhs, const Domain& rhs);

} //namespace Fred::Akm
} //namespace Fred

#endif
