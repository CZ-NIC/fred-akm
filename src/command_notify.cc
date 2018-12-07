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
#include "src/command_notify.hh"

#include "src/domain_state.hh"
#include "src/domain_state_stack.hh"
#include "src/domain_status.hh"
#include "src/domain_united_state.hh"
#include "src/domain_united_state_stack.hh"
#include "src/enum_conversions.hh"
#include "src/log.hh"
#include "src/notification_type.hh"
#include "src/notification.hh"
#include "src/domain_notified_status.hh"
#include "src/utils.hh"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>

#include <time.h>

namespace Fred {
namespace Akm {

namespace {

struct StatsInsecureAkmCandidates {
    int domains_loaded;
    int domains_checked;
    int domains_ok;
    int domains_ko;
    int domains_unknown_no_data;

    int sent_ok_notifications;
    int sent_ko_notifications;
    int sent_first_ok_notifications;
    int sent_first_ko_notifications;
    int not_sent_first_ko_notifications;
    int not_sent_still_ok_notifications;
    int not_sent_still_ko_notifications;
    int sending_notification_failures;
    int sending_notification_not_possible;

    void print()
    {
        log()->info("===== NOTIFY STATS INSECURE CANDIDATES ======");
        log()->info("domains loaded:                      {:>8}", domains_loaded);
        log()->info("domains checked:                     {:>8}", domains_checked);
        log()->info(" ├─ ok:                              {:>8}", domains_ok);
        log()->info(" ├─ ko:                              {:>8}", domains_ko);
        log()->info(" └─ no data:                         {:>8}", domains_unknown_no_data);
        log()->info("---------------------------------------------");
        log()->info("sent notifications:                  {:>8}", sent_ok_notifications +
                                                                  sent_ko_notifications);
        log()->info(" ├─ ok notifications:                {:>8}", sent_ok_notifications);
        log()->info(" │   └─ first ok:                    {:>8}", sent_first_ok_notifications);
        log()->info(" └─ ko notifications:                {:>8}", sent_ko_notifications);
        log()->info("     └─first ko:                     {:>8}", sent_first_ko_notifications);
        log()->info("not sent notifications:              {:>8}", not_sent_still_ok_notifications +
                                                                  not_sent_first_ko_notifications +
                                                                  not_sent_still_ko_notifications);
        log()->info(" ├─ still ok:                        {:>8}", not_sent_still_ok_notifications);
        log()->info(" ├─ first ko:                        {:>8}", not_sent_first_ko_notifications);
        log()->info(" └─ still ko:                        {:>8}", not_sent_still_ko_notifications);
        log()->info("sending notification not possible:   {:>8}", sending_notification_not_possible);
        log()->info("sending notification failures:       {:>8}", sending_notification_failures);
        log()->info("=============================================");
    }
};

StatsInsecureAkmCandidates stats_insecure_akm_candidates;

enum struct DomainStatusChange
{
    new_ok,
    new_ko,
    ok_ok,
    ok_ko,
    ko_ok,
    ko_ko,
    ok_ok2,
};

std::string to_string(const DomainStatusChange& domain_status_change)
{
    switch (domain_status_change)
    {
        case DomainStatusChange::new_ok: return "new->ok"; break;
        case DomainStatusChange::new_ko: return "new->ko"; break;
        case DomainStatusChange::ok_ok: return "ok->ok"; break;
        case DomainStatusChange::ok_ko: return "ok->ko"; break;
        case DomainStatusChange::ko_ok: return "ko->ok"; break;
        case DomainStatusChange::ko_ko: return "ko->ko"; break;
        case DomainStatusChange::ok_ok2: return "ok->ok2"; break;
    }
    throw std::invalid_argument("Fred::Akm::DomainStatusChange value out of range");
}

void update_stats_according_to_domain_status_change(
        StatsInsecureAkmCandidates& stats_insecure_akm_candidates,
        const DomainStatusChange& domain_status_change)
{
    switch (domain_status_change)
    {
        case DomainStatusChange::new_ok:
            stats_insecure_akm_candidates.domains_ok++;
            stats_insecure_akm_candidates.sent_ok_notifications++;
            stats_insecure_akm_candidates.sent_first_ok_notifications++;
            break;

        case DomainStatusChange::new_ko:
            stats_insecure_akm_candidates.domains_ko++;
            stats_insecure_akm_candidates.not_sent_first_ko_notifications++;
            break;

        case DomainStatusChange::ok_ok:
            stats_insecure_akm_candidates.domains_ok++;
            stats_insecure_akm_candidates.not_sent_still_ok_notifications++;
            break;

        case DomainStatusChange::ok_ko:
            stats_insecure_akm_candidates.domains_ko++;
            stats_insecure_akm_candidates.sent_ko_notifications++;
            break;

        case DomainStatusChange::ko_ok:
            stats_insecure_akm_candidates.domains_ok++;
            stats_insecure_akm_candidates.sent_ok_notifications++;
            break;

        case DomainStatusChange::ko_ko:
            stats_insecure_akm_candidates.domains_ko++;
            stats_insecure_akm_candidates.not_sent_still_ko_notifications++;
            break;

        case DomainStatusChange::ok_ok2:
            stats_insecure_akm_candidates.domains_ok++;
            stats_insecure_akm_candidates.sent_ok_notifications++;
            break;
        default:
            throw std::invalid_argument("update_stats_according_to_domain_status_change input value(s) out of range");
    }
}

struct UnknownDomainStatusChange : std::exception
{
    const char* what() const throw ()
    {
        return "unknow domain status change";
    }
};

DomainStatus::DomainStatusType to_new_domain_status_type(const DomainStatusChange& _domain_status_change)
{
    switch (_domain_status_change)
    {
        case DomainStatusChange::new_ok: return DomainStatus::DomainStatusType::akm_status_candidate_ok; break;
        case DomainStatusChange::new_ko: return DomainStatus::DomainStatusType::akm_status_candidate_ko; break;
        case DomainStatusChange::ok_ok: return DomainStatus::DomainStatusType::akm_status_candidate_ok; break;
        case DomainStatusChange::ok_ko: return DomainStatus::DomainStatusType::akm_status_candidate_ko; break;
        case DomainStatusChange::ko_ok: return DomainStatus::DomainStatusType::akm_status_candidate_ok; break;
        case DomainStatusChange::ko_ko: return DomainStatus::DomainStatusType::akm_status_candidate_ko; break;
        case DomainStatusChange::ok_ok2: return DomainStatus::DomainStatusType::akm_status_candidate_ok; break;
    }
    throw std::invalid_argument("Fred::Akm::DomainStatusChange value out of range");
}

DomainStatusChange get_domain_status_change(
        boost::optional<DomainNotifiedStatus> _domain_notified_status,
        const DomainUnitedState& _domain_newest_united_state,
        const int _maximal_time_between_scan_results,
        const int _current_unix_time)
{
    const auto domain_newest_united_state_status =
            (!_domain_newest_united_state.is_coherent() ||
             _domain_newest_united_state.get_cdnskeys().empty() ||
             is_dnssec_turn_off_requested(_domain_newest_united_state))
                    ? DomainStatus::DomainStatusType::akm_status_candidate_ko
                    : DomainStatus::DomainStatusType::akm_status_candidate_ok;

    if (!_domain_notified_status)
    {
        if (domain_newest_united_state_status == DomainStatus::DomainStatusType::akm_status_candidate_ok)
        {
            return DomainStatusChange::new_ok;
        }
        else if (domain_newest_united_state_status == DomainStatus::DomainStatusType::akm_status_candidate_ko)
        {
            return DomainStatusChange::new_ko;
        }
    }
    else {
        if (_domain_notified_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_candidate_ok))
        {
            if (domain_newest_united_state_status == DomainStatus::DomainStatusType::akm_status_candidate_ok)
            {
                const bool domain_states_are_coherent =
                        _domain_notified_status
                                ? are_coherent(_domain_newest_united_state, *_domain_notified_status)
                                : false;
                return domain_states_are_coherent
                        ? DomainStatusChange::ok_ok
                        : DomainStatusChange::ok_ok2;
            }
            else if (domain_newest_united_state_status == DomainStatus::DomainStatusType::akm_status_candidate_ko)
            {
                return DomainStatusChange::ok_ko;
            }
        }
        else if ((_domain_notified_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_candidate_ko)) ||
                (_domain_notified_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_managed_ok))) // fallen angel
        {
            if (domain_newest_united_state_status == DomainStatus::DomainStatusType::akm_status_candidate_ok)
            {
                return DomainStatusChange::ko_ok;
            }
            else if (domain_newest_united_state_status == DomainStatus::DomainStatusType::akm_status_candidate_ko)
            {
                return DomainStatusChange::ko_ko;
            }
        }
    }
    throw UnknownDomainStatusChange();
}

void command_notify_insecure_akm_candidates(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const unsigned long _maximal_time_between_scan_results,
        const unsigned long _minimal_scan_result_sequence_length_to_notify,
        const bool  _notify_from_last_scan_iteration_only,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    auto scan_result_rows =
            _storage.get_scan_result_rows_of_akm_insecure_candidates_for_akm_notify(
                    _minimal_scan_result_sequence_length_to_notify + _maximal_time_between_scan_results,
                    _notify_from_last_scan_iteration_only);

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

    stats_insecure_akm_candidates.domains_loaded = domain_united_state_stack.domains_with_united_states.size();

    const unsigned int current_unix_time = _storage.get_current_unix_time();
    log()->debug("current unix time taken from db: {}", current_unix_time);

    log()->debug(";== [command_notify] =========================================================================");

    for (const auto& domain_with_united_states : domain_united_state_stack.domains_with_united_states)
    {
        const auto& domain = domain_with_united_states.first;
        const auto& domain_united_states = domain_with_united_states.second;

        try
        {
            if (domain_united_states.empty())
            {
                log()->error("no united state for domain {}", domain.fqdn);
                stats_insecure_akm_candidates.domains_unknown_no_data++;
                continue;
            }
            stats_insecure_akm_candidates.domains_checked++;

            log()->debug("domain {}", to_string(domain));

            boost::optional<DomainNotifiedStatus> domain_notified_status =
                    _storage.get_domain_last_notified_status(domain.id);
            log()->debug("last notified status: {}",
                    domain_notified_status ? to_string(*domain_notified_status) : "NOT FOUND");

            bool all_status_changes_worth_to_notify_processed = false;
            while (!all_status_changes_worth_to_notify_processed) // Note: can be dangerous
            {
                DomainUnitedState domain_united_state_to_notify;

                const DomainUnitedState& domain_newest_united_state = domain_united_states.back();
                log()->debug("newest domain_status: {}", to_string(domain_newest_united_state));

                DomainStatusChange domain_status_change =
                        get_domain_status_change(
                                domain_notified_status,
                                domain_newest_united_state,
                                _maximal_time_between_scan_results,
                                current_unix_time);

                log()->debug(to_string(domain_status_change));

                if (domain_status_change == DomainStatusChange::ok_ok) // oh, special case
                {
                    const boost::optional<DomainUnitedState> domain_intermediate_united_state_to_notify =
                            lookup_domain_intermediate_united_state(
                                    domain,
                                    domain_united_states,
                                    _maximal_time_between_scan_results,
                                    std::max<int>(current_unix_time - _maximal_time_between_scan_results - _maximal_time_between_scan_results, domain_notified_status ? domain_notified_status->last_at.scan_seconds : 0));
                    if (domain_intermediate_united_state_to_notify)
                    {
                        domain_united_state_to_notify = *domain_intermediate_united_state_to_notify;
                        domain_status_change = DomainStatusChange::ok_ko;
                        all_status_changes_worth_to_notify_processed = false;
                    }
                    else
                    {
                        domain_united_state_to_notify = domain_newest_united_state;
                        all_status_changes_worth_to_notify_processed = true;
                    }
                }
                else
                {
                    domain_united_state_to_notify = domain_newest_united_state;
                    all_status_changes_worth_to_notify_processed = true;
                }

                const bool domain_status_has_changed =
                        domain_status_change == DomainStatusChange::new_ok ||
                        domain_status_change == DomainStatusChange::ok_ko ||
                        domain_status_change == DomainStatusChange::ko_ok ||
                        domain_status_change == DomainStatusChange::ok_ok2;

                const bool notification_should_be_sent = domain_status_has_changed;

                if (notification_should_be_sent)
                {
                    DomainNotifiedStatus new_domain_notified_status =
                            DomainNotifiedStatus(
                                    domain,
                                    domain_united_state_to_notify,
                                    to_new_domain_status_type(domain_status_change));

                    notify_and_save_domain_status(
                            new_domain_notified_status,
                            _storage,
                            _akm_backend,
                            _mailer_backend,
                            _dry_run,
                            _fake_contact_emails);

                    domain_notified_status = new_domain_notified_status;
                    log()->debug("last notified status now: {}",
                            domain_notified_status ? to_string(*domain_notified_status) : "NOT FOUND");
                }

                update_stats_according_to_domain_status_change(
                        stats_insecure_akm_candidates,
                        domain_status_change);
            }

        }
        catch (const UnknownDomainStatusChange&)
        {
            log()->error("unknow domain status chage for domain: {} ", domain.fqdn);
        }
        catch (const NotificationNotPossible&)
        {
            log()->error("notification not possible for domain domain: {} ", domain.fqdn);
            stats_insecure_akm_candidates.sending_notification_not_possible++;
        }
        catch (const NotificationFailed&)
        {
            log()->error("notification failed for domain domain: {} ", domain.fqdn);
            stats_insecure_akm_candidates.sending_notification_failures++;
        }
    }

    stats_insecure_akm_candidates.print();
}

} // namespace Fred::Akm::{anonymous}

void command_notify(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const unsigned long _maximal_time_between_scan_results,
        const unsigned long _minimal_scan_result_sequence_length_to_notify,
        const bool  _notify_from_last_scan_iteration_only,
        const bool _align_to_start_of_day,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    command_notify_insecure_akm_candidates(
            _storage,
            _akm_backend,
            _mailer_backend,
            _maximal_time_between_scan_results,
            _minimal_scan_result_sequence_length_to_notify,
            _notify_from_last_scan_iteration_only,
            _align_to_start_of_day,
            _dry_run,
            _fake_contact_emails);
}


} // namespace Fred::Akm
} // namespace Fred
