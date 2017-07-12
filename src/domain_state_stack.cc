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

#include "src/domain_state_stack.hh"

#include "src/domain_state.hh"
#include "src/scan_iteration.hh"
#include "src/log.hh"

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


DomainStateStack::DomainStateStack(const ScanResultRows& _scan_result_rows)
{
    boost::optional<DomainState> last_domain_state;
    boost::optional<ScanIteration> scan_iteration;
    for (const auto& r : _scan_result_rows)
    {
        if (!last_domain_state) {
            last_domain_state = DomainState(
                            r.scan_at,
                            r.scan_at_seconds,
                            Domain(r.domain_id, r.domain_name, r.has_keyset),
                            r.nameserver,
                            r.nameserver_ip,
                            r.cdnskey);
        }
        else {
            if (is_from_same_nameserver_ip(r, *last_domain_state))
            {
                last_domain_state->add(r.cdnskey);
            }
            else {
                scan_iterations[*scan_iteration][last_domain_state->domain][last_domain_state->nameserver][last_domain_state->nameserver_ip]
                        .emplace_back(*last_domain_state);
                last_domain_state = DomainState(
                        r.scan_at,
                        r.scan_at_seconds,
                        Domain(r.domain_id, r.domain_name, r.has_keyset),
                        r.nameserver,
                        r.nameserver_ip,
                        r.cdnskey);
            }
        }
        if (!scan_iteration) {
            scan_iteration = ScanIteration(r.scan_iteration_id, r.scan_at, r.scan_at_seconds, "");
        }
        else if (r.scan_iteration_id != scan_iteration->id) {
            scan_iteration = ScanIteration(r.scan_iteration_id, r.scan_at, r.scan_at_seconds, "");
        }
        if (&r == &_scan_result_rows.back())
        {
            scan_iterations[*scan_iteration][last_domain_state->domain][last_domain_state->nameserver][last_domain_state->nameserver_ip]
                    .emplace_back(*last_domain_state);
        }
    }
}

boost::optional<DomainState> get_domain_state_if_domain_nameservers_are_coherent(
        const Domain& _domain,
        const DomainStateStack::Nameservers& _nameservers,
        const int _scan_result_row_timediff_max,
        const int _scan_result_row_sequence_timediff_min)
{
    bool domain_ok = true;
    bool scan_result_row_timediff_max_ok = true;
    bool key_check_ok = true;
    bool scan_result_row_sequence_timediff_min_ok = false;
    int indent = 1;

    boost::optional<DomainState> newest_domain_state; // domain state as reported by its most recently scanned namserver_ip
    for (const auto& nameserver : _nameservers) {
        //indented_print(indent + 1, nameserver.first);
        for (const auto& nameserver_ip : nameserver.second) {
            //indented_print(indent + 2, nameserver_ip.first);
            if (nameserver_ip.second.size()) {
                const auto& domain_state = nameserver_ip.second.front();
                if (!newest_domain_state) {
                    newest_domain_state = domain_state;
                }
                else if (domain_state.scan_at_seconds > newest_domain_state->scan_at_seconds)
                {
                    newest_domain_state = domain_state;
                }
            }
            else {
                log()->info("no IP addresses for nameserver {}", nameserver.first);
                return boost::optional<DomainState>();
            }
        }
    }

    if (newest_domain_state && newest_domain_state->cdnskeys.size()) {
        indented_print(indent + 0, "domain newest: " + to_string(*newest_domain_state));
        indented_print(indent + 0, "");

        for (const auto& nameserver : _nameservers) {
            indented_print(indent + 1, nameserver.first);
            scan_result_row_sequence_timediff_min_ok = false;
            boost::optional<DomainState> last_domain_state = newest_domain_state;
            for (const auto& nameserver_ip : nameserver.second) {
                indented_print(indent + 2, nameserver_ip.first);
                for (const auto& domain_state : nameserver_ip.second) {
                    indented_print(indent + 3, to_string(domain_state));

                    if (!domain_state.cdnskeys.size()) {
                        indented_print(indent + 3, "KEY CHECK KO (no keys ~ insecure-empty)");
                        key_check_ok = false;
                        break;
                    }

                    if (last_domain_state->scan_at_seconds - domain_state.scan_at_seconds
                        > _scan_result_row_timediff_max)
                    {
                        indented_print(indent + 3, "TIMEDIFF CHECK KO ");
                        scan_result_row_timediff_max_ok = false;
                        break;
                    }
                    else {
                        indented_print(indent + 3, "timediff check ok");
                    }

                    if (!are_coherent(domain_state, *newest_domain_state)) {
                        indented_print(indent + 3, "KEY CHECK KO");
                        key_check_ok = false;
                        break;
                    }
                    else {
                        indented_print(indent + 3, "key check ok");
                    }

                    if (newest_domain_state->scan_at_seconds - last_domain_state->scan_at_seconds
                        >= _scan_result_row_sequence_timediff_min)
                    {
                        indented_print(indent + 3, "sequence timediff check reached ok, leaving this ns");
                        scan_result_row_sequence_timediff_min_ok = true;
                        break; // requirements reached, can leave now
                    }
                    else {
                        indented_print(indent + 3, "sequence timediff check pending");
                    }

                    last_domain_state = domain_state;
                }
            }
        }
    }
    else {
        indented_print(indent + 1, "KO, no recent scan result WITH CDNSKEYS for domain " + _domain.fqdn);
        domain_ok = false;
    }

    if (!key_check_ok) {
        domain_ok = false;
    }
    if (!scan_result_row_timediff_max_ok) {
        domain_ok = false;
    }
    if (!scan_result_row_sequence_timediff_min_ok) {
        if (domain_ok) {
            indented_print(indent + 2, "SEQUENCE TIMEDIFF CHECK KO");
        }
        domain_ok = false;
    }

    indented_print(indent + 1, std::string("DOMAIN STATUS for this iteration: ") + (domain_ok ? "OK" : "KO"));

    return domain_ok ? newest_domain_state : boost::optional<DomainState>();
}


void print(const DomainStateStack& _domain_state_stack)
{
    indented_print(0, ";== [DomainStateStack] ==========");
    indented_print(0, "[scan_iteration]", ";");
    indented_print(1, "[domain]", ";");
    indented_print(2, "[nameserver]", ";");
    indented_print(3, "[nameserver_ip]", ";");
    indented_print(4, "[list of domain states (as reported by concrete namserver_ip)]", ";");
    for (const auto& scan_iteration : _domain_state_stack.scan_iterations) {
        indented_print(0, to_string(scan_iteration.first));
        for (const auto& domain : scan_iteration.second) {
            indented_print(1, to_string(domain.first));
            for (const auto& nameserver : domain.second) {
                indented_print(2, nameserver.first);
                for (const auto& nameserver_ip : nameserver.second) {
                    indented_print(3, nameserver_ip.first);
                    for (const auto& domain_state : nameserver_ip.second) {
                        indented_print(4, to_string(domain_state));
                    }
                }
            }
        }
    }
    log()->debug("");
}

void remove_scan_result_rows_from_older_scan_iterations_per_domain(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        auto domain = domains.find(_scan_result_row.domain_id);
                        if (domain != domains.end()) {
                            return _scan_result_row.scan_iteration_id != domain->second;
                        }
                        domains[_scan_result_row.domain_id] = _scan_result_row.scan_iteration_id;
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_insecure(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_insecure(_scan_result_row)) {
                            log()->debug("IGNORING NOT INSECURE scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_insecure_with_data(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_insecure_with_data(_scan_result_row)) {
                            log()->debug("IGNORING NOT INSECURE WITH DATA scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_secure(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_secure(_scan_result_row)) {
                            log()->debug("IGNORING NOT SECURE scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_secure_with_data(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_secure_with_data(_scan_result_row)) {
                            log()->debug("IGNORING NOT SECURE WITH DATA scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_all_scan_result_rows_for_domains_with_some_not_insecure_with_data_scan_result_rows(ScanResultRows& _scan_result_rows)
{
    std::set<unsigned long long> domains_with_invalid_scan_result_rows;
    for (const auto& r : _scan_result_rows)
    {
        if (!is_insecure_with_data(r)) {
            log()->error("SKIPPED NOT INSECURE WITH DATA scan_result_row: {}", to_string(r));
            domains_with_invalid_scan_result_rows.insert(r.domain_id);
            continue;
        }
    }
    for (const auto& domain_with_invalid_scan_result_rows : domains_with_invalid_scan_result_rows)
    {
        log()->error("SKIPPED DOMAIN with invalid scan_result_row(s): {}", domain_with_invalid_scan_result_rows);
    }
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        auto domain = domains_with_invalid_scan_result_rows.find(_scan_result_row.domain_id);
                        if (domain != domains_with_invalid_scan_result_rows.end()) {
                            //log()->error("SKIPPED scan_result_row for DOMAIN with invalid scan_result_row(s): {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(ScanResultRows& _scan_result_rows)
{
    std::set<unsigned long long> domains_with_invalid_scan_result_rows;
    for (const auto& r : _scan_result_rows)
    {
        if (!is_valid(r)) {
            log()->error("SKIPPED INVALID scan_result_row:       {}", to_string(r));
            domains_with_invalid_scan_result_rows.insert(r.domain_id);
            continue;
        }
    }
    for (const auto& domain_with_invalid_scan_result_rows : domains_with_invalid_scan_result_rows)
    {
        log()->error("SKIPPED DOMAIN with invalid scan_result_row(s): {}", domain_with_invalid_scan_result_rows);
    }
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        auto domain = domains_with_invalid_scan_result_rows.find(_scan_result_row.domain_id);
                        if (domain != domains_with_invalid_scan_result_rows.end()) {
                            //log()->error("SKIPPED scan_result_row for DOMAIN with invalid scan_result_row(s): {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

} //namespace Fred::Akm
} //namespace Fred
