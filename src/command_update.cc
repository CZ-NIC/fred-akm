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

#include "src/cdnskey.hh"
#include "src/dnskey.hh"
#include "src/domain_state.hh"
#include "src/domain_state_stack.hh"
#include "src/domain_status_stack.hh"
#include "src/notified_domain_status.hh"
#include "src/keyset.hh"
#include "src/log.hh"
#include "src/nsset.hh"
#include "src/notification.hh"
#include "src/utils.hh"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>

namespace Fred {
namespace Akm {

namespace {

struct Stats {
    int domains_loaded;
    int domains_checked;
    int domains_ok;
    int domains_ok_for_update;
    int domains_ko;
    int domains_ko_for_update_not_all_historic_statuses_ok;
    int domains_ko_for_update_not_all_historic_statuses_coherent;
    int domains_unknown_no_data;

    void print()
    {
        log()->info("================== STATS ====================");
        log()->info("domains loaded:                      {:>8}", domains_loaded);
        log()->info("domains checked:                     {:>8}", domains_checked);
        log()->info("  domains ok:                        {:>8}", domains_ok);
        log()->info("    domains ok_for_update:           {:>8}", domains_ok_for_update);
        log()->info("  domains ko:                        {:>8}", domains_ko);
        log()->info("    domains domains ko for update not all historic statuses ok:");
        log()->info("                                     {:>8}", domains_ko_for_update_not_all_historic_statuses_ok);
        log()->info("    domains domains ko for update not all historic statuses coherent:");
        log()->info("                                     {:>8}", domains_ko_for_update_not_all_historic_statuses_coherent);
        log()->info("    no data:                         {:>8}", domains_unknown_no_data);
        log()->info("---------------------------------------------");
        log()->info("=============================================");
    }
};

Stats stats;

} // namespace Fred::Akm::{anonymous}

void command_update(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        unsigned long _maximal_time_between_scan_results,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool _dry_run)
{
    auto scan_result_rows =
            _storage.get_insecure_scan_result_rows_for_update(
                    _minimal_scan_result_sequence_length_to_update);

    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    remove_scan_result_rows_other_than_insecure(scan_result_rows);
    remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(scan_result_rows);
    log()->debug("removed invalid and possibly other than insecure scan_iterations -> finally remains {} scan result(s)", scan_result_rows.size());

    DomainStateStack domain_state_stack(scan_result_rows);
    print(domain_state_stack);

    for (const auto& scan_iteration : domain_state_stack.scan_iterations) {
        stats.domains_loaded += scan_iteration.second.size();
    }

    DomainStatusStack domain_status_stack(domain_state_stack, _maximal_time_between_scan_results);
    print(domain_status_stack);

    log()->info(";== [command_update] =========================================================================");

    stats.domains_loaded = domain_status_stack.domains.size();
    for (const auto& domain : domain_status_stack.domains) {
        if (domain.second.empty()) {
            log()->error("no statuses for domain {}", domain.first.fqdn);
            stats.domains_unknown_no_data++;
            continue;
        }
        stats.domains_checked++;

        log()->debug(";-- [domain {}] --------------------------", to_string(domain.first));

        boost::optional<NotifiedDomainStatus> notified_domain_status =
                _storage.get_last_notified_domain_status(domain.first.id);

        log()->debug("last notified status: {}: {}",
                notified_domain_status ? to_status_string(*notified_domain_status) : "NOT FOUND",
                notified_domain_status ? to_string(*notified_domain_status) : "-");

        DomainStatus newest_domain_status = domain.second.back();

        log()->debug("newest domain_status: {}: {}",
                to_status_string(newest_domain_status),
                to_string(newest_domain_status));

        if (notified_domain_status && (newest_domain_status.status != notified_domain_status->domain_status)) {
            log()->error("newest domain state != notified domain state; run notify first???");
            continue;
        }

        bool domain_ok = true;
        for (const auto& domain_status : boost::adaptors::reverse(domain.second))
        {
            if (domain_status.status != DomainStatus::akm_status_candidate_ok)
            {
                log()->debug("non-acceptable (not ok) domain status found {}", to_string(domain_status));
                domain_ok = false;
                stats.domains_ko_for_update_not_all_historic_statuses_ok++;
                break;
            }
            if (!are_coherent(*domain_status.domain_state, *notified_domain_status))
            {
                log()->debug("non-acceptable (not coherent with the current one) domain status found {}", to_string(domain_status));
                domain_ok = false;
                stats.domains_ko_for_update_not_all_historic_statuses_coherent++;
                break;
            }

        }

        if (domain_ok)
        {
            stats.domains_ok_for_update++;
            log()->debug("will update domain {}", domain.first.fqdn);

            Nsset current_nsset(newest_domain_status.nameservers);
            log()->debug("current_nsset: {}", to_string(current_nsset));

            Keyset new_keyset;
            for (const auto& cdnskey : newest_domain_status.domain_state->cdnskeys)
            {
                //log()->debug("cdnskey: {}", to_string(cdnskey.second));
                new_keyset.dnskeys.push_back(
                        Dnskey(
                                cdnskey.second.flags,
                                cdnskey.second.proto,
                                cdnskey.second.alg,
                                cdnskey.second.public_key));
            }
            log()->debug("new_keyset: {}", to_string(new_keyset));

            try {
                if (!_dry_run) {
                    log()->debug("UPDATE domain {}", domain.first.fqdn);
                    _akm_backend.update_domain_automatic_keyset(domain.first.id, current_nsset, new_keyset); // FIXME
                    log()->debug("UPDATE OK for domain {}", domain.first.fqdn);
                }
            }
            catch(std::runtime_error& e)
            {
                log()->debug("UPDATE FAILED for domain {}", domain.first.fqdn);
                log()->debug(e.what());
                continue;
            }
            catch(...)
            {
                log()->debug("UPDATE FAILED for domain {}", domain.first.fqdn);
                throw;
            }
            newest_domain_status.status = DomainStatus::akm_status_managed_ok;
            NotifiedDomainStatus new_notified_domain_status =
                    NotifiedDomainStatus(
                            domain.first,
                            newest_domain_status);
            /* no notification for non-akm -> akm change, domain update notification goes from backend
            notify_and_save_domain_status(
                    new_notified_domain_status,
                    _storage,
                    _akm_backend,
                    _mailer_backend,
                    _dry_run);
            */
            _storage.set_notified_domain_status(new_notified_domain_status);
        }

    }

    stats.print();
    log()->debug("command update done");
}

} //namespace Fred::Akm
} //namespace Fred
