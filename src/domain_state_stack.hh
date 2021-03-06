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
#ifndef DOMAIN_STATE_STACK_HH_13945F8D78934FF6A7CFE9B6A6CBAC06
#define DOMAIN_STATE_STACK_HH_13945F8D78934FF6A7CFE9B6A6CBAC06

#include "src/cdnskey.hh"
#include "src/scan_iteration.hh"
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
    typedef std::string Nameserver;
    typedef std::string NameserverIp;

    typedef std::vector<DomainState> DomainStates; // TODO: there will be just one state per iteration now, vector not needed
    typedef std::map<NameserverIp, DomainStates> NameserverIps;
    typedef std::map<Nameserver, NameserverIps> Nameservers;
    typedef std::map<Domain, Nameservers, DomainLexicographicalComparator> Domains;
    typedef std::map<ScanIteration, Domains> ScanIterations;

    DomainStateStack(const ScanResultRows& _scan_result_rows);

    ScanIterations scan_iterations;
};

void print(const DomainStateStack& _domain_state_stack);

bool operator<(const Domain& lhs, const Domain& rhs);

void remove_scan_result_rows_from_older_scan_iterations_per_domain(ScanResultRows& _scan_result_rows);
void remove_scan_result_rows_other_than_insecure(ScanResultRows& _scan_result_rows);
void remove_scan_result_rows_other_than_insecure_with_data(ScanResultRows& _scan_result_rows);
void remove_scan_result_rows_other_than_secure_auto(ScanResultRows& _scan_result_rows);
void remove_scan_result_rows_other_than_secure_auto_with_data(ScanResultRows& _scan_result_rows);
void remove_scan_result_rows_other_than_secure_noauto(ScanResultRows& _scan_result_rows);
void remove_scan_result_rows_other_than_secure_noauto_with_data(ScanResultRows& _scan_result_rows);
void remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(ScanResultRows& _scan_result_rows);
void remove_all_scan_result_rows_for_domains_with_some_not_insecure_with_data_scan_result_rows(ScanResultRows& _scan_result_rows);

} //namespace Fred::Akm
} //namespace Fred

#endif
