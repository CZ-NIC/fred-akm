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

#ifndef DOMAIN_STATUS_STACK_HH_0DF9FFB1B9B54D9DA39E857EF0E6FC73
#define DOMAIN_STATUS_STACK_HH_0DF9FFB1B9B54D9DA39E857EF0E6FC73

#include "src/domain_state.hh"
#include "src/domain_state_stack.hh"
#include "src/domain_status.hh"

namespace Fred {
namespace Akm {

struct DomainStatusStack {
    typedef std::vector<DomainStatus> DomainStatuses;
    typedef std::map<Domain, DomainStatuses> Domains;

    DomainStatusStack(const DomainStateStack& _domain_state_stack, unsigned long _maximal_time_between_scan_results);

    Domains domains;
};

void print(const DomainStatusStack& _domain_status_stack);

} //namespace Fred::Akm
} //namespace Fred

#endif
