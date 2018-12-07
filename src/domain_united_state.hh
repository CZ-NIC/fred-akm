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
#ifndef DOMAIN_UNITED_STATE_HH_CA8752A6DBD045648BD639AE3497F8F1
#define DOMAIN_UNITED_STATE_HH_CA8752A6DBD045648BD639AE3497F8F1

#include "src/domain.hh"
#include "src/domain_state.hh"
#include "src/scan_date_time.hh"
#include "src/scan_iteration.hh"

#include <boost/optional.hpp>

#include <set>
#include <vector>

namespace Fred {
namespace Akm {

struct InvalidDomainUnitedState
{
};

class DomainUnitedState
{
public:
    DomainUnitedState()
        : domain(),
          scan_iteration()
    {
    }

    DomainUnitedState(
            const Domain& _domain,
            const ScanIteration& _scan_iteration)
        : domain(_domain),
          scan_iteration(_scan_iteration)
    {
    }

    void add(const DomainState& domain_state)
    {
        domain_states.push_back(domain_state);
    }

    bool is_coherent() const
    {
        for (std::vector<DomainState>::const_iterator domain_state = domain_states.begin();
             domain_state != domain_states.end();
             ++domain_state)
        {
            if (!are_coherent(*domain_states.begin(), *domain_state))
            {
                return false;
            }
        }
        return true;
    }

    bool is_empty() const
    {
        return domain_states.empty();
    }

    ScanDateTime get_scan_from() const
    {
        if (is_empty())
        {
            //throw InvalidDomainUnitedState();
            return ScanDateTime();
        }

        std::vector<DomainState>::const_iterator domain_state = domain_states.begin();
        ScanDateTime scan_from = domain_state->scan_at;
        while (++domain_state != domain_states.end())
        {
            if (domain_state->scan_at < scan_from)
            {
                scan_from = domain_state->scan_at;
            }
        }
        return scan_from;
    }

    ScanDateTime get_scan_to() const
    {
        if (is_empty())
        {
            //throw InvalidDomainUnitedState();
            return ScanDateTime();
        }

        std::vector<DomainState>::const_iterator domain_state = domain_states.begin();
        ScanDateTime scan_to = domain_state->scan_at;
        while (++domain_state != domain_states.end())
        {
            if (domain_state->scan_at > scan_to)
            {
                scan_to = domain_state->scan_at;
            }
        }
        return scan_to;
    }

    std::map<std::string, Cdnskey> get_cdnskeys() const
    {
        if (is_empty() || !is_coherent())
        {
            //throw InvalidDomainUnitedState();
            return std::map<std::string, Cdnskey>();
        }

        return domain_states.begin()->cdnskeys;
    }

    std::set<std::string> get_nameservers() const
    {
        std::set<std::string> nameservers;
        for (std::vector<DomainState>::const_iterator domain_state = domain_states.begin();
             domain_state != domain_states.end();
             ++domain_state)
        {
            nameservers.insert(domain_state->nameserver);
        }
        return nameservers;
    }

    Domain domain;
    ScanIteration scan_iteration;

private:
    std::vector<DomainState> domain_states;
};

std::string to_string(const DomainUnitedState& _domain_united_state, bool verbose = false);

bool is_narrow(
        const DomainUnitedState& _domain_united_state,
        int seconds_max);

bool is_recent(
        const DomainUnitedState& _domain_united_state,
        int seconds_max,
        int current_unix_time);

bool are_close(
        const DomainUnitedState& _first_domain_united_state,
        const DomainUnitedState& _second_domain_united_state,
        int seconds_max);

bool are_coherent(
        const DomainUnitedState& _first_domain_united_state,
        const DomainUnitedState& _second_domain_united_state);

bool is_dnssec_turn_off_requested(const DomainUnitedState& _domain_united_state);

} // namespace Fred::Akm
} // namespace Fred

#endif
