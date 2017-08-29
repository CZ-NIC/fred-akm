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

struct DomainIdWithIterationId
{


    DomainIdWithIterationId(
            const unsigned long long _domain_id,
            const unsigned long long _scan_iteration_id)
        : domain_id(_domain_id),
          scan_iteration_id(_scan_iteration_id)
    {
    }


    unsigned long long domain_id;
    unsigned long long scan_iteration_id;

};

bool operator<(const DomainIdWithIterationId& _lhs, const DomainIdWithIterationId& _rhs)
{
    if (_lhs.domain_id != _rhs.domain_id)
    {
        return _lhs.domain_id < _rhs.domain_id;
    }
    return _lhs.scan_iteration_id < _rhs.scan_iteration_id;
}


} // namespace Fred::Akm::{anonymous}


DomainStateStack::DomainStateStack(const ScanResultRows& _scan_result_rows)
{
    boost::optional<DomainState> last_domain_state;
    boost::optional<ScanIteration> scan_iteration;
    for (const auto& r : _scan_result_rows)
    {
        if (!last_domain_state)
        {
            last_domain_state = DomainState(
                            r.scan_at,
                            Domain(r.domain_id, r.domain_name, r.scan_type),
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
                        Domain(r.domain_id, r.domain_name, r.scan_type),
                        r.nameserver,
                        r.nameserver_ip,
                        r.cdnskey);
            }
        }
        if (!scan_iteration)
        {
            scan_iteration = ScanIteration(r.scan_iteration_id, r.scan_at, ScanDateTime());
        }
        else if (r.scan_iteration_id != scan_iteration->id)
        {
            scan_iteration = ScanIteration(r.scan_iteration_id, r.scan_at, ScanDateTime());
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
        const boost::optional<int> _current_unix_time)
{
    bool recent_state_present = false;
    bool scan_result_row_timediff_max_ok = true;
    bool key_check_ok = true;
    int indent = 1;

    boost::optional<DomainState> domain_newest_state; // domain state as reported by its most recently scanned namserver_ip
    for (const auto& nameserver : _nameservers) {
        //indented_print(indent + 1, nameserver.first);
        for (const auto& nameserver_ip : nameserver.second) {
            //indented_print(indent + 2, nameserver_ip.first);
            if (nameserver_ip.second.size())
            {
                const auto& domain_state = nameserver_ip.second.front();
                if (!domain_newest_state)
                {
                    domain_newest_state = domain_state;
                }
                else if (domain_state.scan_at > domain_newest_state->scan_at)
                {
                    domain_newest_state = domain_state;
                }
            }
            else {
                log()->info("no IP addresses for nameserver {}", nameserver.first);
                return boost::optional<DomainState>();
            }
        }
    }

    if (domain_newest_state && domain_newest_state->cdnskeys.size())
    {
        recent_state_present = true;
        indented_print(indent + 0, "domain newest state: " + to_string(*domain_newest_state));
        indented_print(indent + 0, "");

        for (const auto& nameserver : _nameservers) {
            indented_print(indent + 1, nameserver.first);
            boost::optional<DomainState> last_domain_state = domain_newest_state;
            for (const auto& nameserver_ip : nameserver.second) {
                indented_print(indent + 2, nameserver_ip.first);
                for (const auto& domain_state : nameserver_ip.second) {
                    indented_print(indent + 3, to_string(domain_state));

                    if (!domain_state.cdnskeys.size())
                    {
                        indented_print(indent + 3, "KEY CHECK KO (no keys - is empty)");
                        key_check_ok = false;
                        break;
                    }

                    if (_current_unix_time && (*_current_unix_time - last_domain_state->scan_at.scan_seconds > _scan_result_row_timediff_max))
                    {
                        indented_print(indent + 3, "TIMEDIFF CHECK KO (too old)");
                        scan_result_row_timediff_max_ok = false;
                        break;
                    }

                    if (last_domain_state->scan_at.scan_seconds - domain_state.scan_at.scan_seconds
                        > _scan_result_row_timediff_max)
                    {
                        indented_print(indent + 3, "TIMEDIFF CHECK KO ");
                        scan_result_row_timediff_max_ok = false;
                        break;
                    }
                    else {
                        indented_print(indent + 3, "timediff check ok");
                    }

                    if (!are_coherent(domain_state, *domain_newest_state))
                    {
                        indented_print(indent + 3, "KEY CHECK KO");
                        key_check_ok = false;
                        break;
                    }
                    else {
                        indented_print(indent + 3, "key check ok");
                    }

                    last_domain_state = domain_state;
                }
            }
        }
    }
    else {
        indented_print(indent + 1, "KO, no recent scan result WITH CDNSKEYS for domain " + _domain.fqdn);
    }

    const bool domain_ok = recent_state_present && key_check_ok && scan_result_row_timediff_max_ok;

    indented_print(indent + 1, std::string("DOMAIN STATUS for this iteration: ") + (domain_ok ? "OK" : "KO"));

    return domain_ok ? domain_newest_state : boost::optional<DomainState>();
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
                        if (domain != domains.end())
                        {
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
                        if (!is_insecure(_scan_result_row))
                        {
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
                        if (!is_insecure_with_data(_scan_result_row))
                        {
                            log()->debug("IGNORING NOT INSECURE WITH DATA scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_secure_auto(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_secure_auto(_scan_result_row))
                        {
                            log()->debug("IGNORING NOT SECURE AUTO scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_secure_auto_with_data(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_secure_auto_with_data(_scan_result_row))
                        {
                            log()->debug("IGNORING NOT SECURE AUTO WITH DATA scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_secure_noauto(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_secure_noauto(_scan_result_row))
                        {
                            log()->debug("IGNORING NOT SECURE NOAUTO scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_scan_result_rows_other_than_secure_noauto_with_data(ScanResultRows& _scan_result_rows)
{
    std::map<unsigned long long, int> domains;
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        if (!is_secure_noauto_with_data(_scan_result_row))
                        {
                            log()->debug("IGNORING NOT SECURE NOAUTO WITH DATA scan_result_row: {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_all_scan_result_rows_for_domains_with_some_not_insecure_with_data_scan_result_rows(ScanResultRows& _scan_result_rows)
{
    std::set<DomainIdWithIterationId> domains_with_invalid_scan_result_rows;
    for (const auto& r : _scan_result_rows)
    {
        if (!is_insecure_with_data(r))
        {
            log()->error("SKIPPED NOT INSECURE WITH DATA scan_result_row: {}", to_string(r));
            domains_with_invalid_scan_result_rows.insert(DomainIdWithIterationId(r.domain_id, r.scan_iteration_id));
            continue;
        }
    }
    for (const auto& domain_with_invalid_scan_result_rows : domains_with_invalid_scan_result_rows)
    {
        log()->error("SKIPPED DOMAIN with not insecure with data scan_result_row(s): {} in iteration {}", domain_with_invalid_scan_result_rows.domain_id, domain_with_invalid_scan_result_rows.scan_iteration_id);
    }
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        auto domain = domains_with_invalid_scan_result_rows.find(DomainIdWithIterationId(_scan_result_row.domain_id, _scan_result_row.scan_iteration_id));
                        if (domain != domains_with_invalid_scan_result_rows.end())
                        {
                            //log()->error("SKIPPED scan_result_row for DOMAIN with invalid scan_result_row(s): {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

void remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(ScanResultRows& _scan_result_rows)
{
    std::set<DomainIdWithIterationId> domains_with_invalid_scan_result_rows;
    for (const auto& r : _scan_result_rows)
    {
        if (!is_valid(r))
        {
            log()->error("SKIPPED INVALID scan_result_row:       {}", to_string(r));
            domains_with_invalid_scan_result_rows.insert(DomainIdWithIterationId(r.domain_id, r.scan_iteration_id));
            continue;
        }
    }
    for (const auto& domain_with_invalid_scan_result_rows : domains_with_invalid_scan_result_rows)
    {
        log()->error("SKIPPED DOMAIN with invalid scan_result_row(s): {} in iteration {}", domain_with_invalid_scan_result_rows.domain_id, domain_with_invalid_scan_result_rows.scan_iteration_id);
    }
    _scan_result_rows.erase(
            std::remove_if(
                    _scan_result_rows.begin(),
                    _scan_result_rows.end(),
                    [&](const ScanResultRow& _scan_result_row)
                    {
                        auto domain = domains_with_invalid_scan_result_rows.find(DomainIdWithIterationId(_scan_result_row.domain_id, _scan_result_row.scan_iteration_id));
                        if (domain != domains_with_invalid_scan_result_rows.end())
                        {
                            //log()->error("SKIPPED scan_result_row for DOMAIN with invalid scan_result_row(s): {}", to_string(_scan_result_row));
                            return true;
                        }
                        return false;
                    }),
            _scan_result_rows.end());
}

} //namespace Fred::Akm
} //namespace Fred
