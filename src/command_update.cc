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
#include "src/domain_united_state.hh"
#include "src/domain_united_state_stack.hh"
#include "src/domain_notified_status.hh"
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

struct StatsInsecureCandidates {
    int domains_loaded;
    int domains_checked;
    int domains_ok_for_update;
    int domains_updated_ok;
    int domains_updated_ko;
    int domains_ko_for_update_run_notify_first;
    int domains_ko_for_update_no_recent_scan;
    int domains_ko_for_update_not_all_historic_statuses_ok;
    int domains_ko_for_update_not_all_historic_statuses_coherent;
    int domains_unknown_no_data;

    StatsInsecureCandidates()
        : domains_loaded(),
          domains_checked(),
          domains_ok_for_update(),
          domains_ko_for_update_run_notify_first(),
          domains_ko_for_update_not_all_historic_statuses_ok(),
          domains_ko_for_update_not_all_historic_statuses_coherent(),
          domains_unknown_no_data()
    {
    }

    void print()
    {
        log()->info("======== UPDATE STATS INSECURE CANDIDATES ========");
        log()->info("domains loaded:                          {:>8}", domains_loaded);
        log()->info("domains checked:                         {:>8}", domains_checked);
        log()->info(" ├─ ok_for_update:                       {:>8}", domains_ok_for_update);
        log()->info(" │   ├─ updated:                         {:>8}", domains_updated_ok);
        log()->info(" │   └─ failed:                          {:>8}", domains_updated_ko);
        log()->info(" └─ ko_for_update:                       {:>8}", domains_ko_for_update_run_notify_first +
                                                                      domains_ko_for_update_no_recent_scan +
                                                                      domains_ko_for_update_not_all_historic_statuses_ok +
                                                                      domains_ko_for_update_not_all_historic_statuses_coherent +
                                                                      domains_unknown_no_data);
        log()->info("     ├─ run notify first:                {:>8}", domains_ko_for_update_run_notify_first);
        log()->info("     ├─ no recent scan:                  {:>8}", domains_ko_for_update_no_recent_scan);
        log()->info("     ├─ not all hist. statuses ok:       {:>8}", domains_ko_for_update_not_all_historic_statuses_ok);
        log()->info("     ├─ not all hist. statuses coherent: {:>8}", domains_ko_for_update_not_all_historic_statuses_coherent);
        log()->info("     └─ no data:                         {:>8}", domains_unknown_no_data);
        log()->info("==================================================");
    }
};

struct StatsSecureCandidates {
    int domains_loaded;
    int domains_checked;
    int domains_ok_for_update;
    int domains_updated_ok;
    int domains_updated_ko;
    int domains_ko_for_update_no_recent_scan;
    int domains_unknown_no_data;
    int domains_ko_for_update_no_keys;

    StatsSecureCandidates()
        : domains_loaded(),
          domains_checked(),
          domains_ok_for_update(),
          domains_unknown_no_data(),
          domains_ko_for_update_no_keys()
    {
    }

    void print()
    {
        log()->info("========= UPDATE STATS SECURE CANDIDATES =========");
        log()->info("domains loaded:                          {:>8}", domains_loaded);
        log()->info("domains checked:                         {:>8}", domains_checked);
        log()->info(" ├─ ok_for_update:                       {:>8}", domains_ok_for_update);
        log()->info(" │   ├─ updated:                         {:>8}", domains_updated_ok);
        log()->info(" │   └─ failed:                          {:>8}", domains_updated_ko);
        log()->info(" └─ ko_for_update:                       {:>8}", domains_ko_for_update_no_recent_scan +
                                                                      domains_ko_for_update_no_keys +
                                                                      domains_unknown_no_data);
        log()->info("     ├─ no recent scan:                  {:>8}", domains_ko_for_update_no_recent_scan);
        log()->info("     ├─ no keys:                         {:>8}", domains_ko_for_update_no_keys);
        log()->info("     └─ no data:                         {:>8}", domains_unknown_no_data);
        log()->info("==================================================");
    }
};

struct StatsMembers {
    int domains_loaded;
    int domains_checked;
    int domains_ok_for_update;
    int domains_updated_ok;
    int domains_updated_ko;
    int domains_ko_for_update_same_keys;
    int domains_ko_for_update_no_keys;

    StatsMembers()
        : domains_loaded(),
          domains_checked(),
          domains_ok_for_update()
    {
    }

    void print()
    {
        log()->info("============== UPDATE STATS MEMBERS =============");
        log()->info("domains loaded:                          {:>8}", domains_loaded);
        log()->info("domains checked:                         {:>8}", domains_checked);
        log()->info(" ├─ ok_for_update:                       {:>8}", domains_ok_for_update);
        log()->info(" │   ├─ updated:                         {:>8}", domains_updated_ok);
        log()->info(" │   └─ failed:                          {:>8}", domains_updated_ko);
        log()->info(" └─ ko_for_update:                       {:>8}", domains_ko_for_update_same_keys + domains_ko_for_update_no_keys);
        log()->info("     ├─ same cdnskeys:                   {:>8}", domains_ko_for_update_same_keys);
        log()->info("     └─ no cdnskeys:                     {:>8}", domains_ko_for_update_no_keys);
        log()->info("=================================================");
    }
};


StatsInsecureCandidates stats_akm_insecure_candidates;

void command_update_turn_on_akm_on_insecure_candidates(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        unsigned long _maximal_time_between_scan_results,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    log()->debug(";== [command_update (akm insecure candidates)] ===========================================================");

    auto scan_result_rows =
            _storage.get_scan_result_rows_of_akm_insecure_candidates_for_akm_turn_on(
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

    DomainUnitedStateStack domain_united_state_stack(domain_state_stack);
    print(domain_united_state_stack);

    stats_akm_insecure_candidates.domains_loaded = domain_united_state_stack.domains_with_united_states.size();

    const unsigned int current_unix_time = _storage.get_current_unix_time();
    log()->debug("current unix time taken from db: {}", current_unix_time);

    for (const auto& domain_with_united_states : domain_united_state_stack.domains_with_united_states)
    {
        const auto& domain = domain_with_united_states.first;
        const auto& domain_united_states = domain_with_united_states.second;

        if (domain_united_states.empty())
        {
            log()->error("no united state for domain {}", domain.fqdn);
            stats_akm_insecure_candidates.domains_unknown_no_data++;
            continue;
        }
        stats_akm_insecure_candidates.domains_checked++;

        log()->debug("domain {}", to_string(domain));

        boost::optional<DomainNotifiedStatus> domain_notified_status =
                _storage.get_domain_last_notified_status(domain.id);
        log()->debug("last notified status: {}",
                domain_notified_status ? to_string(*domain_notified_status) : "NOT FOUND");

        //DomainStatus domain_newest_status = domain_united_states.back();
        //log()->debug("newest domain status: {}", to_string(domain_newest_status));

        const DomainUnitedState& domain_newest_united_state = domain_united_states.back();
        log()->debug("newest domain_status: {}", to_string(domain_newest_united_state));


        const bool domain_newest_state_is_recent = !domain_newest_united_state.is_empty() && (current_unix_time - domain_newest_united_state.get_scan_to().scan_seconds > _maximal_time_between_scan_results);
        if (!domain_newest_state_is_recent)
        {
            log()->error("WILL NOT UPDATE domain {}: domain latest state too old", domain.fqdn);
            stats_akm_insecure_candidates.domains_ko_for_update_run_notify_first++;
            continue;
        }

        const bool domain_notified_state_is_ok = domain_notified_status && (domain_notified_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_candidate_ok));
        if (!domain_notified_state_is_ok)
        {
            log()->error("WILL NOT UPDATE domain {}: domain notified state != OK", domain.fqdn);
            stats_akm_insecure_candidates.domains_ko_for_update_run_notify_first++;
            continue;
        }

        const bool domain_notified_state_is_newest_state = domain_notified_status && (are_coherent(domain_newest_united_state, *domain_notified_status));
        if (!domain_notified_state_is_newest_state)
        {
            log()->error("WILL NOT UPDATE domain {}: domain newest state != domain notified state; run notify first???", domain.fqdn);
            stats_akm_insecure_candidates.domains_ko_for_update_run_notify_first++;
            continue;
        }

        const boost::optional<DomainUnitedState> domain_intermediate_united_state_to_not_turn_on =
                lookup_domain_intermediate_united_state(
                        domain,
                        domain_united_states,
                        _maximal_time_between_scan_results);

        const bool domain_history_ok =
                (domain_intermediate_united_state_to_not_turn_on &&
                 current_unix_time >= domain_intermediate_united_state_to_not_turn_on->get_scan_from().scan_seconds && // TODO fix poor unsigned handling someday
                 (current_unix_time - domain_intermediate_united_state_to_not_turn_on->get_scan_from().scan_seconds >= _minimal_scan_result_sequence_length_to_update))
                ||
                (!domain_intermediate_united_state_to_not_turn_on &&
                 current_unix_time >= domain_united_states.front().get_scan_from().scan_seconds && // TODO fix poor unsigned handling someday
                 (current_unix_time - domain_united_states.front().get_scan_from().scan_seconds >= _minimal_scan_result_sequence_length_to_update));

        const bool domain_ok =
                domain_newest_state_is_recent &&
                domain_notified_state_is_ok &&
                domain_notified_state_is_newest_state &&
                domain_history_ok;

        if (domain_ok)
        {
            stats_akm_insecure_candidates.domains_ok_for_update++;
            log()->debug("will update domain {}", domain.fqdn);

            Nsset current_nsset(domain_newest_united_state.get_nameservers());
            log()->debug("current_nsset: {}", to_string(current_nsset));

            Keyset new_keyset;
            for (const auto& cdnskey : domain_newest_united_state.get_cdnskeys())
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

            log()->info("UPDATE domain {} with {} as seen at {}", domain.fqdn, to_string(new_keyset), to_string(current_nsset));
            try
            {
                if (!_dry_run)
                {
                    _akm_backend.turn_on_automatic_keyset_management_on_insecure_domain(domain.id, current_nsset, new_keyset);
                    log()->debug("UPDATE OK for insecure domain {}", domain.fqdn);
                    stats_akm_insecure_candidates.domains_updated_ok++;
                    const auto domain_newest_united_state_status = DomainStatus::DomainStatusType::akm_status_managed_ok;
                    DomainNotifiedStatus new_domain_notified_status =
                            DomainNotifiedStatus(
                                    domain,
                                    domain_newest_united_state,
                                    domain_newest_united_state_status);
                    log()->debug("first update from insecure to secure: not sending any notification/template for domain {}", domain.fqdn);
                    save_domain_status(new_domain_notified_status, _storage, _dry_run);
                }
            }
            // TODO stats
            //catch (const Fred::Akm::ObjectNotFound& e)
            //{
            //    log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            //    stats_akm_insecure_candidates.domains_updated_ko_object_not_found++;
            //    log()->debug(e.what());
            //    continue;
            //}
            catch (const Fred::Akm::AkmException& e)
            {
                log()->error("UPDATE FAILED for domain {}", domain.fqdn);
                stats_akm_insecure_candidates.domains_updated_ko++;
                log()->debug(e.what());
                continue;
            }
            catch (const std::runtime_error& e)
            {
                log()->error("UPDATE FAILED for domain {}", domain.fqdn);
                stats_akm_insecure_candidates.domains_updated_ko++;
                log()->debug(e.what());
                continue;
            }
            catch (...)
            {
                log()->error("UPDATE FAILED for domain {}", domain.fqdn);
                stats_akm_insecure_candidates.domains_updated_ko++;
                throw;
            }
        }

    }
}

StatsSecureCandidates stats_akm_secure_candidates;

void command_update_turn_on_akm_on_secure_candidates(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        unsigned long _maximal_time_between_scan_results,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    log()->debug(";== [command_update (akm secure candidates)] ===========================================================");

    auto scan_result_rows =
            _storage.get_scan_result_rows_of_akm_secure_candidates_for_akm_turn_on(
                    _maximal_time_between_scan_results,
                    _align_to_start_of_day);

    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(scan_result_rows);
    log()->debug("removed domains with invalid scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_scan_result_rows_other_than_secure_noauto_with_data(scan_result_rows);
    log()->debug("removed scan_result_rows with other than secure_noauto_with_data scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_scan_result_rows_from_older_scan_iterations_per_domain(scan_result_rows);
    log()->debug("removed scan_result_rows from older scan iterations than the latest one per domain -> remains {} scan result(s)", scan_result_rows.size());

    DomainStateStack domain_state_stack(scan_result_rows);
    print(domain_state_stack);

    DomainUnitedStateStack domain_united_state_stack(domain_state_stack);
    print(domain_united_state_stack);

    stats_akm_secure_candidates.domains_loaded = domain_united_state_stack.domains_with_united_states.size();

    for (const auto& domain_with_united_states : domain_united_state_stack.domains_with_united_states)
    {
        const auto& domain = domain_with_united_states.first;
        const auto& domain_united_states = domain_with_united_states.second;

        stats_akm_secure_candidates.domains_checked++;

        log()->debug("domain {}", to_string(domain));

        const DomainUnitedState& domain_newest_united_state = domain_united_states.back();

        log()->debug("newest domain united state: {}", to_string(domain_newest_united_state));

        if(!domain_newest_united_state.is_empty() && domain_newest_united_state.get_cdnskeys().empty())
        {
            log()->debug("will not update domain {}, no cdnskeys", domain.fqdn);
            stats_akm_secure_candidates.domains_ko_for_update_no_keys++;
            continue;
        }

        /*
        boost::optional<DomainNotifiedStatus> domain_notified_status =
                _storage.get_domain_last_notified_status(domain.id);

        log()->debug("last notified status: {}",
                domain_notified_status ? to_string(*domain_notified_status) : "NOT FOUND");

        if (domain_notified_status)
        {
            stats_akm_secure_candidates.fallen_angel++;
        }
        */

        const std::string serialized_new_keys = serialize(domain_newest_united_state.get_cdnskeys());
        log()->info("UPDATE domain {} with {} as seen at DNSSEC VALIDATING RESOLVER", domain.fqdn, serialized_new_keys);
        stats_akm_secure_candidates.domains_ok_for_update++;

        try
        {
            if (!_dry_run)
            {
                Keyset new_keyset;
                for (const auto& cdnskey : domain_newest_united_state.get_cdnskeys())
                {
                    new_keyset.dnskeys.push_back(
                            Dnskey(
                                    cdnskey.second.flags,
                                    cdnskey.second.proto,
                                    cdnskey.second.alg,
                                    cdnskey.second.public_key));
                }

                _akm_backend.turn_on_automatic_keyset_management_on_secure_domain(domain.id, new_keyset);
                log()->debug("UPDATE OK for secure noauto domain {}", domain.fqdn);
                stats_akm_secure_candidates.domains_updated_ok++;

                DomainStatus::DomainStatusType domain_newest_united_state_status = DomainStatus::DomainStatusType::akm_status_managed_ok;
                DomainNotifiedStatus new_domain_notified_status =
                        DomainNotifiedStatus(
                                domain,
                                domain_newest_united_state,
                                domain_newest_united_state_status);
                save_domain_status(new_domain_notified_status, _storage, _dry_run);
            }
        }
        // TODO stats
        //catch (const Fred::Akm::ObjectNotFound& e)
        //{
        //    log()->error("UPDATE FAILED for domain {}", domain.fqdn);
        //    stats_akm_secure_candidates.domains_updated_ko_object_not_found++;
        //    log()->debug(e.what());
        //    continue;
        //}
        catch (const Fred::Akm::AkmException& e)
        {
            log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            stats_akm_secure_candidates.domains_updated_ko++;
            log()->debug(e.what());
            continue;
        }
        catch (const std::runtime_error& e)
        {
            log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            stats_akm_secure_candidates.domains_updated_ko++;
            log()->debug(e.what());
            continue;
        }
        catch (...)
        {
            log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            stats_akm_secure_candidates.domains_updated_ko++;
            throw;
        }

    }
}

StatsMembers stats_akm_members;

void command_update_update_akm_members(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        unsigned long _minimal_scan_result_sequence_length_to_update,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    log()->debug(";== [command_update (secure members)] =========================================================================");

    auto scan_result_rows =
            _storage.get_scan_result_rows_of_akm_members_for_update(
                    _minimal_scan_result_sequence_length_to_update,
                    _align_to_start_of_day);

    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    remove_all_scan_result_rows_for_domains_with_some_invalid_scan_result_rows(scan_result_rows);
    log()->debug("removed domains with invalid scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_scan_result_rows_other_than_secure_auto_with_data(scan_result_rows);
    log()->debug("removed scan_result_rows with other than secure_with_data scan_results_rows -> remains {} scan result(s)", scan_result_rows.size());
    remove_scan_result_rows_from_older_scan_iterations_per_domain(scan_result_rows);
    log()->debug("removed scan_result_rows from older scan iterations than the latest one per domain -> remains {} scan result(s)", scan_result_rows.size());

    DomainStateStack domain_state_stack(scan_result_rows);
    print(domain_state_stack);

    DomainUnitedStateStack domain_united_state_stack(domain_state_stack);
    print(domain_united_state_stack);

    stats_akm_members.domains_loaded = domain_united_state_stack.domains_with_united_states.size();

    for (const auto& domain_with_united_states : domain_united_state_stack.domains_with_united_states)
    {
        const auto& domain = domain_with_united_states.first;
        const auto& domain_united_states = domain_with_united_states.second;

        stats_akm_members.domains_checked++;

        log()->debug("domain {}", to_string(domain));

        const DomainUnitedState& domain_newest_united_state = domain_united_states.back();

        log()->debug("newest domain united state: {}", to_string(domain_newest_united_state));

        if(!domain_newest_united_state.is_empty() && domain_newest_united_state.get_cdnskeys().empty())
        {
            log()->debug("will not update domain {}, no cdnskeys", domain.fqdn);
            stats_akm_members.domains_ko_for_update_no_keys++;
            continue;
        }

        boost::optional<DomainNotifiedStatus> domain_notified_status =
                _storage.get_domain_last_notified_status(domain.id);

        log()->debug("last notified status: {}",
                domain_notified_status ? to_string(*domain_notified_status) : "NOT FOUND (AKM administratively set?)");

        const std::string serialized_new_keys = serialize(domain_newest_united_state.get_cdnskeys());
        if (domain_notified_status && (domain_notified_status->serialized_cdnskeys == serialized_new_keys))
        {
            log()->debug("will not update domain {} with {}, keys are identical", domain.fqdn, serialized_new_keys);
            stats_akm_members.domains_ko_for_update_same_keys++;
            continue;
        }

        log()->info("UPDATE domain {} with {} as seen at DNSSEC VALIDATING RESOLVER", domain.fqdn, serialized_new_keys);
        stats_akm_members.domains_ok_for_update++;

        try
        {
            if (!_dry_run)
            {
                Keyset new_keyset;
                for (const auto& cdnskey : domain_newest_united_state.get_cdnskeys())
                {
                    new_keyset.dnskeys.push_back(
                            Dnskey(
                                    cdnskey.second.flags,
                                    cdnskey.second.proto,
                                    cdnskey.second.alg,
                                    cdnskey.second.public_key));
                }

                _akm_backend.update_automatically_managed_keyset_of_domain(domain.id, new_keyset);
                log()->debug("UPDATE OK for secure domain {}", domain.fqdn);
                stats_akm_members.domains_updated_ok++;

                DomainStatus::DomainStatusType domain_newest_united_state_status = DomainStatus::DomainStatusType::akm_status_managed_ok;
                DomainNotifiedStatus new_domain_notified_status =
                        DomainNotifiedStatus(
                                domain,
                                domain_newest_united_state,
                                domain_newest_united_state_status);
                if (!is_dnssec_turn_off_requested(domain_newest_united_state))
                {
                    // do notification of akm change because automatic keyset update notification goes from backend to automatic keyset sponsoring registrar only
                    notify_and_save_domain_status(
                            new_domain_notified_status,
                            _storage,
                            _akm_backend,
                            _mailer_backend,
                            _dry_run,
                            _fake_contact_emails);
                }
                else
                {
                    log()->debug("has_deletekey: not sending any notification/template for domain {}", domain.fqdn);
                    save_domain_status(new_domain_notified_status, _storage, _dry_run);
                }
            }
        }
        // TODO stats
        //catch (const Fred::Akm::ObjectNotFound& e)
        //{
        //    log()->error("UPDATE FAILED for domain {}", domain.fqdn);
        //    stats_akm_members.domains_updated_ko_object_not_found++;
        //    log()->debug(e.what());
        //    continue;
        //}
        catch (const Fred::Akm::AkmException& e)
        {
            log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            stats_akm_members.domains_updated_ko++;
            log()->debug(e.what());
            continue;
        }
        catch (const std::runtime_error& e)
        {
            log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            stats_akm_members.domains_updated_ko++;
            log()->debug(e.what());
            continue;
        }
        catch (...)
        {
            log()->error("UPDATE FAILED for domain {}", domain.fqdn);
            stats_akm_members.domains_updated_ko++;
            throw;
        }

    }
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
    command_update_turn_on_akm_on_insecure_candidates(
            _storage,
            _akm_backend,
            _maximal_time_between_scan_results,
            _minimal_scan_result_sequence_length_to_update,
            _align_to_start_of_day,
            _dry_run,
            _fake_contact_emails);

    command_update_turn_on_akm_on_secure_candidates(
            _storage,
            _akm_backend,
            _maximal_time_between_scan_results,
            _align_to_start_of_day,
            _dry_run,
            _fake_contact_emails);

    command_update_update_akm_members(
            _storage,
            _akm_backend,
            _mailer_backend,
            _minimal_scan_result_sequence_length_to_update,
            _align_to_start_of_day,
            _dry_run,
            _fake_contact_emails);

    stats_akm_insecure_candidates.print();
    stats_akm_secure_candidates.print();
    stats_akm_members.print();
}

} //namespace Fred::Akm
} //namespace Fred
