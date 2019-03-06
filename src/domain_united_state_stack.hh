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
#ifndef DOMAIN_UNITED_STATE_STACK_HH_833AD3BC218746F684ADBF3A0F03A1DB
#define DOMAIN_UNITED_STATE_STACK_HH_833AD3BC218746F684ADBF3A0F03A1DB

#include "src/domain_united_state.hh"
#include "src/domain_state_stack.hh"

namespace Fred {
namespace Akm {

struct DomainUnitedStateStack {
    typedef std::vector<DomainUnitedState> DomainUnitedStates;
    typedef std::map<Domain, DomainUnitedStates> DomainsWithUnitedStates;

    DomainUnitedStateStack(const DomainStateStack& _domain_state_stack);

    DomainsWithUnitedStates domains_with_united_states;
};

void print(const DomainUnitedStateStack& _domain_united_state_stack);

boost::optional<DomainUnitedState> lookup_domain_intermediate_united_state(
        const Domain& domain,
        const DomainUnitedStateStack::DomainUnitedStates& _domain_united_states,
        unsigned long long _maximal_time_between_scan_results,
        int _current_unix_time);

} //namespace Fred::Akm
} //namespace Fred

#endif
