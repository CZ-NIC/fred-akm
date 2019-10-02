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
#include "src/domain_united_state_stack.hh"

#include "src/domain_united_state.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {

namespace {
    void indented_print(const int _indent, const std::string& _message, const std::string& _prefix = "")
    {
        std::string result = _prefix;
        for (int i = 0; i < _indent; ++i)
        {
            result += "    ";
        }
        result += _message;
        log()->debug(result);
    }
} // namespace Fred::Akm::{anonymous}

DomainUnitedStateStack::DomainUnitedStateStack(const DomainStateStack& _domain_state_stack)
{
    indented_print(0, ";== [DomainUnitedStateStack: Recognizing domain united states per iteration] =============");
    indented_print(0, "[iteration]", ";");
    indented_print(1, "[domain]", ";");
    indented_print(1, "domain newest state (keycheck compares scan results with this one)", ";");
    indented_print(2, "[nameserver]", ";");
    indented_print(3, "[nameserver_ip]", ";");
    indented_print(4, "[list of domain states (as reported by concrete nameserver_ip)]", ";");
    indented_print(4, "timediff check", ";");
    indented_print(4, "key check", ";");
    indented_print(2, "domain united state for this iteration", ";");
    for (const auto& scan_iteration_with_domains : _domain_state_stack.scan_iterations)
    {
        const auto& scan_iteration = scan_iteration_with_domains.first;
        const auto& domains = scan_iteration_with_domains.second;

        indented_print(0, to_string(scan_iteration));
        for (const auto& domain_with_nameservers : domains)
        {
            const auto& domain = domain_with_nameservers.first;
            const auto& nameservers = domain_with_nameservers.second;

            indented_print(1, to_string(domain));
            if (nameservers.empty())
            {
                indented_print(2, "no scan_iterations for domain " + domain.fqdn);
                continue;
            }

            DomainUnitedState domain_united_state(domain, scan_iteration);

            int indent = 1;
            for (const auto& nameserver : nameservers) {
                indented_print(indent + 1, nameserver.first);
                for (const auto& nameserver_ip : nameserver.second) {
                    indented_print(indent + 2, nameserver_ip.first);
                    for (const auto& domain_state : nameserver_ip.second) {
                        indented_print(indent + 3, to_string(domain_state));
                        domain_united_state.add(domain_state);
                    }
                }
            }

            indented_print(indent, to_string(domain_united_state));

            domains_with_united_states[domain].push_back(domain_united_state);
        }
    }
}

void print(const DomainUnitedStateStack& _domain_united_state_stack)
{
    indented_print(0, ";== [DomainUnitedStateStack] ==========");
    indented_print(0, "[domain]", ";");
    indented_print(1, "[list of domain united states (chronologically, one from each scan_iteration where (if) the domain was scanned)]", ";");
    for (const auto& domain : _domain_united_state_stack.domains_with_united_states)
    {
        indented_print(0, domain.first.fqdn);
        for (const auto& domain_united_state : domain.second)
        {
            indented_print(1, to_string(domain_united_state));
        }
    }
    log()->debug("");
}

boost::optional<DomainUnitedState> lookup_domain_intermediate_united_state(
        const Domain& domain,
        const DomainUnitedStateStack::DomainUnitedStates& _domain_united_states,
        const unsigned long long _maximal_time_between_scan_results,
        const long long _seconds_back)
{
    auto domain_checked_united_state = _domain_united_states.rbegin();
    auto* domain_last_checked_united_state = &*domain_checked_united_state;
    while (++domain_checked_united_state != _domain_united_states.rend())
    {
        if (!domain_last_checked_united_state->is_coherent() ||
            !are_coherent(
                    *domain_last_checked_united_state,
                    *domain_checked_united_state) ||
            !are_close(
                    *domain_last_checked_united_state,
                    *domain_checked_united_state,
                    _maximal_time_between_scan_results))
        {
            log()->debug("ok->...(ko)...->ok (ko_found@scan_iteration {})", to_string(domain_last_checked_united_state->scan_iteration));
            return *domain_last_checked_united_state;
        }
        domain_last_checked_united_state = &*domain_checked_united_state;
        if (domain_checked_united_state + 1 == _domain_united_states.rend())
        {
            if (!domain_last_checked_united_state->is_empty() &&
                (domain_checked_united_state->get_scan_to().scan_seconds > _seconds_back) &&
                (static_cast<unsigned long long>(domain_checked_united_state->get_scan_to().scan_seconds - _seconds_back) >
                 _maximal_time_between_scan_results))
            {
            log()->error("domain {} oldest state not continuous {} - {} = {} > {}",
                    domain.fqdn,
                    domain_checked_united_state->get_scan_to().scan_seconds,
                    _seconds_back,
                    (domain_checked_united_state->get_scan_to().scan_seconds - _seconds_back),
                    _maximal_time_between_scan_results);
                return *domain_last_checked_united_state;
            };
        }
    }
    return boost::optional<DomainUnitedState>();
}

} // namespace Fred::Akm
} // namespace
