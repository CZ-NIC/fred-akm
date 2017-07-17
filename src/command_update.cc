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

struct StatsInsecure {
    int domains_loaded;
    int domains_checked;
    int domains_ok;
    int domains_ok_for_update;
    int domains_updated_ok;
    int domains_updated_ko;
    int domains_ko;
    int domains_ko_for_update_run_notify_first;
    int domains_ko_for_update_not_all_historic_statuses_ok;
    int domains_ko_for_update_not_all_historic_statuses_coherent;
    int domains_unknown_no_data;

    StatsInsecure()
        : domains_loaded(),
          domains_checked(),
          domains_ok(),
          domains_ok_for_update(),
          domains_ko(),
          domains_ko_for_update_run_notify_first(),
          domains_ko_for_update_not_all_historic_statuses_ok(),
          domains_ko_for_update_not_all_historic_statuses_coherent(),
          domains_unknown_no_data()
    {
    }

    void print()
    {
        log()->info("========== UPDATE STATS INSECURE ============");
        log()->info("domains loaded:                      {:>8}", domains_loaded);
        log()->info("domains checked:                     {:>8}", domains_checked);
        log()->info("  domains ok:                        {:>8}", domains_ok);
        log()->info("    domains ok_for_update:           {:>8}", domains_ok_for_update);
        log()->info("      domains updated:               {:>8}", domains_updated_ok);
        log()->info("      domains failed:                {:>8}", domains_updated_ko);
        log()->info("  domains ko:                        {:>8}", domains_ko);
        log()->info("    domains ko_for_update (run notify first):");
        log()->info("                                     {:>8}", domains_ko_for_update_run_notify_first);
        log()->info("    domains ko_for_update (not all historic statuses ok):");
        log()->info("                                     {:>8}", domains_ko_for_update_not_all_historic_statuses_ok);
        log()->info("    domains ko_for_update (not all historic statuses coherent):");
        log()->info("                                     {:>8}", domains_ko_for_update_not_all_historic_statuses_coherent);
        log()->info("    no data:                         {:>8}", domains_unknown_no_data);
        log()->info("=============================================");
    }
};

struct StatsSecure {
    int domains_loaded;
    int domains_checked;
    int domains_for_update;
    int domains_updated_ok;
    int domains_updated_ko;
    int domains_not_for_update_same_keys;
    int domains_not_for_update_no_keys;

    StatsSecure()
        : domains_loaded(),
          domains_checked(),
          domains_for_update()
    {
    }

    void print()
    {
        log()->info("========== UPDATE STATS SECURE ===================");
        log()->info("domains loaded:                           {:>8}", domains_loaded);
        log()->info("domains checked:                          {:>8}", domains_checked);
        log()->info("  domains for_update:                     {:>8}", domains_for_update);
        log()->info("    domains updated:                      {:>8}", domains_updated_ok);
        log()->info("    domains failed:                       {:>8}", domains_updated_ko);
        log()->info("  domains not for update (same cdnskeys): {:>8}", domains_not_for_update_same_keys);
        log()->info("  domains not for update (no cdnskeys):   {:>8}", domains_not_for_update_no_keys);
        log()->info("==================================================");
    }
};


StatsInsecure stats_insecure;
StatsSecure stats_secure;

void command_update_insecure(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        unsigned long _maximal_time_between_scan_results,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    log()->info(";== [command_update (insecure)] =========================================================================");

    auto scan_result_rows =
            _storage.get_insecure_scan_result_rows_for_update(
                    _minimal_scan_result_sequence_length_to_update,
                    _align_to_start_of_day);

    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    remove_scan_result_rows_other_than_insecure(scan_result_rows);
    log()->debug("removed other than insecure scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_all_scan_result_rows_for_domains_with_some_not_insecure_with_data_scan_result_rows(scan_result_rows);
    log()->debug("removed domains with other than insecure_with_data scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(scan_result_rows);
    log()->debug("removed domains with invalid scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());

    DomainStateStack domain_state_stack(scan_result_rows);
    print(domain_state_stack);

    for (const auto& scan_iteration : domain_state_stack.scan_iterations) {
        stats_insecure.domains_loaded += scan_iteration.second.size();
    }

    DomainStatusStack domain_status_stack(domain_state_stack, _maximal_time_between_scan_results);
    print(domain_status_stack);

    log()->info(";== command_update (insecure) data ready");
    stats_insecure.domains_loaded = domain_status_stack.domains.size();
    for (const auto& domain : domain_status_stack.domains) {
        if (domain.second.empty()) {
            log()->error("no statuses for domain {}", domain.first.fqdn);
            stats_insecure.domains_unknown_no_data++;
            continue;
        }
        stats_insecure.domains_checked++;

        log()->debug(";-- [domain {}] --------------------------", to_string(domain.first));

        boost::optional<NotifiedDomainStatus> notified_domain_status =
                _storage.get_last_notified_domain_status(domain.first.id);

        log()->debug("last notified status: {}",
                notified_domain_status ? to_string(*notified_domain_status) : "NOT FOUND");

        DomainStatus newest_domain_status = domain.second.back();

        log()->debug("newest domain status: {}", to_string(newest_domain_status));

        if (notified_domain_status && (newest_domain_status.status != notified_domain_status->domain_status)) {
            log()->error("newest domain state != notified domain state; run notify first???");
            stats_insecure.domains_ko++;
            stats_insecure.domains_ko_for_update_run_notify_first++;
            continue;
        }

        bool domain_ok = true;
        stats_insecure.domains_ok++;
        for (const auto& domain_status : boost::adaptors::reverse(domain.second))
        {
            if (domain_status.status != DomainStatus::akm_status_candidate_ok)
            {
                log()->debug("non-acceptable (not ok) domain status found {}", to_string(domain_status));
                domain_ok = false;
                stats_insecure.domains_ko++;
                stats_insecure.domains_ko_for_update_not_all_historic_statuses_ok++;
                break;
            }
            if (!are_coherent(*domain_status.domain_state, *notified_domain_status))
            {
                log()->debug("non-acceptable (not coherent with the current one) domain status found {}", to_string(domain_status));
                domain_ok = false;
                stats_insecure.domains_ko++;
                stats_insecure.domains_ko_for_update_not_all_historic_statuses_coherent++;
                break;
            }

        }

        if (domain_ok)
        {
            stats_insecure.domains_ok_for_update++;
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

            log()->debug("UPDATE domain {} with {} as seen at {}", domain.first.fqdn, to_string(new_keyset), to_string(current_nsset));
            try {
                if (!_dry_run) {
                    _akm_backend.update_domain_automatic_keyset(domain.first.id, current_nsset, new_keyset); // FIXME
                    log()->debug("UPDATE OK for insecure domain {}", domain.first.fqdn);
                    stats_insecure.domains_updated_ok++;
                    newest_domain_status.status = DomainStatus::akm_status_managed_ok;
                    NotifiedDomainStatus new_notified_domain_status =
                            NotifiedDomainStatus(
                                    domain.first,
                                    newest_domain_status);
                    log()->debug("first update from insecure to secure: not sending any notification/template for domain {}", domain.first.fqdn);
                    save_domain_status(new_notified_domain_status, _storage, _dry_run);
                }
            }
            catch(std::runtime_error& e)
            {
                log()->debug("UPDATE FAILED for domain {}", domain.first.fqdn);
                stats_insecure.domains_updated_ko++;
                log()->debug(e.what());
                continue;
            }
            catch(...)
            {
                log()->debug("UPDATE FAILED for domain {}", domain.first.fqdn);
                stats_insecure.domains_updated_ko++;
                throw;
            }
        }

    }

    log()->info(";== command update (insecure) done");
}

void command_update_secure(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    log()->info(";== [command_update (secure)] =========================================================================");

    auto scan_result_rows =
            _storage.get_secure_scan_result_rows_for_update(
                    _minimal_scan_result_sequence_length_to_update,
                    _align_to_start_of_day);

    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(scan_result_rows);
    log()->debug("removed domains with invalid scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_scan_result_rows_other_than_secure_with_data(scan_result_rows);
    log()->debug("removed scan_result_rows with other than secure_with_data scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_scan_result_rows_from_older_scan_iterations_per_domain(scan_result_rows);
    log()->debug("removed scan_result_rows from older scan iterations than the latest one per domain -> remains {} scan result(s)", scan_result_rows.size());

    DomainStateStack domain_state_stack(scan_result_rows);
    print(domain_state_stack);

    for (const auto& scan_iteration : domain_state_stack.scan_iterations) {
        stats_secure.domains_loaded += scan_iteration.second.size();
    }

    //DomainStatusStack domain_status_stack(domain_state_stack, _minimal_scan_result_sequence_length_to_update);
    //print(domain_status_stack);
    std::map<Domain, DomainState> domains;
    for (const auto& scan_iteration : boost::adaptors::reverse(domain_state_stack.scan_iterations)) {
        for (const auto& domain : scan_iteration.second) {
            for (const auto& nameserver : domain.second) {
                for (const auto& nameserver_ip : nameserver.second) {
                    for (const auto& domain_state : nameserver_ip.second) { // TODO is just one state per domain/iteration, loop/vector not needed
                        //log()->debug(to_string(domain_state));
                        domains[domain_state.domain] = domain_state;
                    }
                }
            }
        }
    }

    log()->info(";== command_update (secure) data ready");

    for (const auto& domain : domains) {

        stats_secure.domains_checked++;

        Nsset current_nsset;
        const DomainStatus newest_domain_status(DomainStatus::akm_status_managed_ok, ScanIteration(), domain.second, current_nsset.nameservers);

        log()->debug("newest domain_status: {}", to_string(newest_domain_status));

        if(newest_domain_status.domain_state->cdnskeys.empty())
        {
            log()->debug("will not update domain {}, no cdnskeys", domain.first.fqdn);
            stats_secure.domains_not_for_update_no_keys++;
            continue;
        }

        boost::optional<NotifiedDomainStatus> notified_domain_status =
                _storage.get_last_notified_domain_status(domain.first.id);

        log()->debug("last notified status: {}",
                notified_domain_status ? to_string(*notified_domain_status) : "NOT FOUND");


        const std::string serialized_new_keys = serialize(newest_domain_status.domain_state->cdnskeys);
        if(serialized_new_keys == notified_domain_status->serialized_cdnskeys)
        {
            log()->debug("will not update domain {} with {}, keys are identical", domain.first.fqdn, serialized_new_keys);
            stats_secure.domains_not_for_update_same_keys++;
            continue;
        }

        log()->debug("UPDATE domain {} with {} as seen at DNSSEC VALIDATING RESOLVER", domain.first.fqdn, serialized_new_keys);
        stats_secure.domains_for_update++;

        try {
            if (!_dry_run) {

                Keyset new_keyset;
                for (const auto& cdnskey : newest_domain_status.domain_state->cdnskeys)
                {
                    new_keyset.dnskeys.push_back(
                            Dnskey(
                                    cdnskey.second.flags,
                                    cdnskey.second.proto,
                                    cdnskey.second.alg,
                                    cdnskey.second.public_key));
                }

                _akm_backend.update_domain_automatic_keyset(domain.first.id, Nsset(), new_keyset); // FIXME
                log()->debug("UPDATE OK for secure domain {}", domain.first.fqdn);
                stats_secure.domains_updated_ok++;
                NotifiedDomainStatus new_notified_domain_status =
                        NotifiedDomainStatus(
                                domain.first,
                                newest_domain_status);
                if (!has_deletekey(*newest_domain_status.domain_state)) {
                    // do notification of akm change because automatic keyset update notification goes from backend to automatic keyset sponsoring registrar only
                    notify_and_save_domain_status(
                            new_notified_domain_status,
                            _storage,
                            _akm_backend,
                            _mailer_backend,
                            _dry_run,
                            _fake_contact_emails);
                }
                else {
                    log()->debug("has_deletekey: not sending any notification/template for domain {}", domain.first.fqdn);
                    save_domain_status(new_notified_domain_status, _storage, _dry_run);
                }
            }
        }
        catch(std::runtime_error& e)
        {
            log()->debug("UPDATE FAILED for domain {}", domain.first.fqdn);
            stats_secure.domains_updated_ko++;
            log()->debug(e.what());
            continue;
        }
        catch(...)
        {
            log()->debug("UPDATE FAILED for domain {}", domain.first.fqdn);
            stats_secure.domains_updated_ko++;
            throw;
        }

    }

    log()->info(";== command update (secure) done");
}

} // namespace Fred::Akm::{anonymous}

void command_update(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const unsigned long _maximal_time_between_scan_results,
        const unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    command_update_insecure(
            _storage,
            _akm_backend,
            _maximal_time_between_scan_results,
            _minimal_scan_result_sequence_length_to_update,
            _align_to_start_of_day,
            _dry_run,
            _fake_contact_emails);

    command_update_secure(
            _storage,
            _akm_backend,
            _mailer_backend,
            _minimal_scan_result_sequence_length_to_update,
            _align_to_start_of_day,
            _dry_run,
            _fake_contact_emails);

    stats_insecure.print();
    stats_secure.print();
}

} //namespace Fred::Akm
} //namespace Fred
