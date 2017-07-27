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

#include "src/domain_status_stack.hh"

#include "src/domain_status.hh"
#include "src/log.hh"
#include "src/notified_domain_status.hh"
#include "src/scan_iteration.hh"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace Fred {
namespace Akm {

namespace {
    void indented_print(const int _indent, const std::string& _message, const std::string& _prefix = "")
    {
        std::string result = _prefix;
        for (int i = 0; i < _indent; ++i) {
            result += "    ";
        }
        result += _message;
        log()->debug(result);
    }
} // namespace Fred::Akm::{anonymous}


DomainStatusStack::DomainStatusStack(const DomainStateStack& _domain_state_stack, unsigned long _maximal_time_between_scan_results)
{
    indented_print(0, ";== [DomainStatusStack: Recognizing domain statuses per iteration] =============");
    indented_print(0, "[iteration]", ";");
    indented_print(1, "[domain]", ";");
    indented_print(1, "domain newest state (keycheck compares scan results with this one)", ";");
    indented_print(2, "[nameserver]", ";");
    indented_print(3, "[nameserver_ip]", ";");
    indented_print(4, "[list of domain states (as reported by concrete nameserver_ip)]", ";");
    indented_print(4, "timediff check", ";");
    indented_print(4, "key check", ";");
    indented_print(4, "timediff sequence check", ";");
    indented_print(2, "DOMAIN STATUS for this iteration", ";");
    for (const auto& scan_iteration : _domain_state_stack.scan_iterations) {
        indented_print(0, to_string(scan_iteration.first));
        for (const auto& domain : scan_iteration.second) {
            indented_print(1, to_string(domain.first));
            if (scan_iteration.second.empty()) {
                indented_print(2, "no scan_iterations for domain " + domain.first.fqdn);
                continue;
            }
            boost::optional<DomainState> iteration_domain_state =
                    get_domain_state_if_domain_nameservers_are_coherent(
                            domain.first,
                            domain.second,
                            _maximal_time_between_scan_results,
                            0);

            const bool is_iteration_domain_state_with_deletekey = iteration_domain_state && has_deletekey(*iteration_domain_state);

            const DomainStatus::DomainStatusType domain_status =
                    iteration_domain_state && !is_iteration_domain_state_with_deletekey
                            ? DomainStatus::DomainStatusType::akm_status_candidate_ok
                            : DomainStatus::DomainStatusType::akm_status_candidate_ko;

            if (domains.find(domain.first) == domains.end()) {
                domains[domain.first] = DomainStatuses();
            }
            std::vector<std::string> nameservers;
            boost::copy(domain.second | boost::adaptors::map_keys, std::back_inserter(nameservers));
            domains[domain.first].push_back(DomainStatus(domain_status, scan_iteration.first, iteration_domain_state, nameservers));
        }
    }
}

void print(const DomainStatusStack& _domain_status_stack)
{
    indented_print(0, ";== [DomainStatusStack] ==========");
    indented_print(0, "[domain]", ";");
    indented_print(1, "[list of domain statuses (chronologically, one from each scan_iteration where (if) the domain was scanned)]", ";");
    for (const auto& domain : _domain_status_stack.domains) {
        indented_print(0, domain.first.fqdn);
        for (const auto& domain_status : domain.second) {
            indented_print(1, to_string(domain_status));
        }
    }
    log()->debug("");
}

} //namespace Fred::Akm
} //namespace Fred
