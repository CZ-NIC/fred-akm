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

#include "src/command_update.hh"

#include "src/domain_state.hh"
#include "src/domain_state_stack.hh"
#include "src/log.hh"
#include "src/utils.hh"

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>

namespace Fred {
namespace Akm {

void command_update(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        unsigned long _maximal_time_between_scan_results,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool  _notify_from_last_iteration_only,
        const bool _dry_run)
{
    auto scan_results = _storage.get_insecure_scan_result_rows(_minimal_scan_result_sequence_length_to_update, _notify_from_last_iteration_only); // FIXME
    log()->debug("got from database {} scan result(s))", scan_results.size());

    DomainStateStack haystack(scan_results);

    print(haystack);

    for (const auto& domain : haystack.domains) {
        log()->debug(domain.first.fqdn);
        bool domain_nameservers_coherent;
        boost::optional<DomainState> newest_domain_state =
                get_last_domain_state(
                        domain.first,
                        domain.second,
                        _maximal_time_between_scan_results,
                        _minimal_scan_result_sequence_length_to_update,
                        domain_nameservers_coherent);

        if (domain_nameservers_coherent) {

            log()->debug("NOT YET IMPLEMENTED");
            //_akm_backend.update
        }

    }

    log()->debug("done");
}

} //namespace Fred::Akm
} //namespace Fred
