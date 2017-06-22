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
    void indented_print(const int _indent, const std::string& _message)
    {
        std::string result = "";
        for (int i = 0; i < _indent; ++i) {
            result += "\t";
        }
        result += _message;
        log()->debug(result);
    }
} // namespace Fred::Akm::{anonymous}


DomainStateStack::DomainStateStack(const ScanResultRows& _scan_result_rows)
{
    std::set<std::string> domains_with_invalid_scan_result_rows;

    boost::optional<DomainState> last_domain_state;
    for (const auto& r : _scan_result_rows)
    {
        if (!is_insecure_with_data(r))
        {
            log()->debug("not importing scan_result_row: {}", to_string(r));
        }
        if (is_valid(r)) {
            log()->debug("importig scan_result_row: {}", to_string(r));
        }
        else {
            log()->error("SKIPPED INVALID scan_result_row: {}", to_string(r));
            domains_with_invalid_scan_result_rows.insert(r.domain_name);
            continue;
        }

        if (!last_domain_state) {
            last_domain_state = DomainState(
                            r.scan_at,
                            r.scan_at_seconds,
                            r.domain_id,
                            r.domain_name,
                            r.has_keyset,
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
                domains[Domain(last_domain_state->domain_id, last_domain_state->domain_name, false)][last_domain_state->nameserver][last_domain_state->nameserver_ip]
                        .emplace_back(*last_domain_state);
                last_domain_state = DomainState(
                        r.scan_at,
                        r.scan_at_seconds,
                        r.domain_id,
                        r.domain_name,
                        r.has_keyset,
                        r.nameserver,
                        r.nameserver_ip,
                        r.cdnskey);
            }
        }
        if (&r == &_scan_result_rows.back())
        {
            domains[Domain(last_domain_state->domain_id, last_domain_state->domain_name, false)][last_domain_state->nameserver][last_domain_state->nameserver_ip]
                    .emplace_back(*last_domain_state);
        }
    }

    auto domain = domains.begin();
    while(domain != domains.end())
    {
        if (domains_with_invalid_scan_result_rows.find(domain->first.fqdn) != domains_with_invalid_scan_result_rows.end())
        {
            log()->error("ERROR: domain \"{}\" has invalid scan_result(s), skipping this domain", domain->first.fqdn);
            domain = domains.erase(domain);
        }
        else
        {
            ++domain;
        }
    }
}

boost::optional<DomainState> get_last_domain_state_if_domain_nameservers_are_coherent(
        const Domain& _domain,
        const DomainStateStack::Nameservers& _nameservers,
        const int _scan_result_row_timediff_max,
        const int _scan_result_row_sequence_timediff_min)
{
    bool domain_ok = true;
    bool scan_result_row_timediff_max_ok = true;
    bool key_check_ok = true;
    bool scan_result_row_sequence_timediff_min_ok = false;

    boost::optional<DomainState> newest_domain_state; // domain state as reported by its most recently scanned namserver_ip
    for (const auto& nameserver : _nameservers) {
        //indented_print(1, nameserver.first);
        for (const auto& nameserver_ip : nameserver.second) {
            //indented_print(2, nameserver_ip.first);
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
        indented_print(0, "domain newest: " + to_string(*newest_domain_state));
        indented_print(0, "-------------' ");

        for (const auto& nameserver : _nameservers) {
            indented_print(1, nameserver.first);
            scan_result_row_sequence_timediff_min_ok = false;
            boost::optional<DomainState> last_domain_state = newest_domain_state;
            for (const auto& nameserver_ip : nameserver.second) {
                indented_print(2, nameserver_ip.first);
                for (const auto& domain_state : nameserver_ip.second) {
                    indented_print(3, to_string(domain_state));

                    if (!domain_state.cdnskeys.size()) {
                        indented_print(3, "KEY CHECK KO (no keys ~ insecure-empty)");
                        key_check_ok = false;
                        break;
                    }

                    if (last_domain_state->scan_at_seconds - domain_state.scan_at_seconds
                        > _scan_result_row_timediff_max)
                    {
                        indented_print(3, "TIMEDIFF CHECK KO ");
                        scan_result_row_timediff_max_ok = false;
                        break;
                    }
                    else {
                        indented_print(3, "timediff check ok");
                    }

                    if (!are_coherent(domain_state, *newest_domain_state)) {
                        indented_print(3, "KEY CHECK KO");
                        key_check_ok = false;
                        break;
                    }
                    else {
                        indented_print(3, "key check ok");
                    }

                    if (newest_domain_state->scan_at_seconds - last_domain_state->scan_at_seconds
                        >= _scan_result_row_sequence_timediff_min)
                    {
                        indented_print(3, "sequence timediff check reached ok, leaving this ns");
                        scan_result_row_sequence_timediff_min_ok = true;
                        break; // requirements reached, can leave now
                    }
                    else {
                        indented_print(3, "sequence timediff check pending");
                    }

                    last_domain_state = domain_state;
                }
            }
        }
    }
    else {
        indented_print(1, "KO, no recent scan result WITH CDNSKEYS for domain " + _domain.fqdn);
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
            indented_print(2, "SEQUENCE TIMEDIFF CHECK KO");
        }
        domain_ok = false;
    }

    indented_print(1, std::string("DOMAIN CHECK ") + (domain_ok ? "OK" : "KO"));

    return domain_ok ? newest_domain_state : boost::optional<DomainState>();
}


void print(const DomainStateStack& _haystack)
{
    for (const auto& domain : _haystack.domains) {
        indented_print(0, domain.first.fqdn);
        for (const auto& nameserver : domain.second) {
            indented_print(1, nameserver.first);
            for (const auto& nameserver_ip : nameserver.second) {
                indented_print(2, nameserver_ip.first);
                for (const auto& scan_result_row : nameserver_ip.second) {
                    indented_print(3, to_string(scan_result_row));
                }
            }
        }
    }
    log()->debug("");
}

bool operator<(const Domain& _lhs, const Domain& _rhs)
{
    return _lhs.fqdn < _rhs.fqdn;
}

} //namespace Fred::Akm
} //namespace Fred
