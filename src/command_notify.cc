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

#include "src/command_notify.hh"

#include "src/domain_state.hh"
#include "src/domain_state_stack.hh"
#include "src/domain_status.hh"
#include "src/domain_status_stack.hh"
#include "src/enum_conversions.hh"
#include "src/log.hh"
#include "src/notification_type.hh"
#include "src/notification.hh"
#include "src/notified_domain_status.hh"
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

struct Stats {
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

    void print()
    {
        log()->info("=============== NOTIFY STATS ================");
        log()->info("domains loaded:                      {:>8}", domains_loaded);
        log()->info("domains checked:                     {:>8}", domains_checked);
        log()->info(" ├─ ok:                              {:>8}", domains_ok);
        log()->info(" ├─ ko:                              {:>8}", domains_ko);
        log()->info(" └─ no data:                         {:>8}", domains_unknown_no_data);
        log()->info("---------------------------------------------");
        log()->info("sent notifications:                  {:>8}", sent_ok_notifications + sent_ko_notifications);
        log()->info(" ├─ ok notifications:                {:>8}", sent_ok_notifications);
        log()->info(" │   └─ first ok:                    {:>8}", sent_first_ok_notifications);
        log()->info(" └─ ko notifications:                {:>8}", sent_ko_notifications);
        log()->info("     └─first ko:                     {:>8}", sent_first_ko_notifications);
        log()->info("not sent notifications:              {:>8}", not_sent_still_ok_notifications + not_sent_first_ko_notifications + not_sent_still_ko_notifications);
        log()->info(" ├─ still ok:                        {:>8}", not_sent_still_ok_notifications);
        log()->info(" ├─ first ko:                        {:>8}", not_sent_first_ko_notifications);
        log()->info(" └─ still ko:                        {:>8}", not_sent_still_ko_notifications);
        log()->info("sending notification failures:       {:>8}", sending_notification_failures);
        log()->info("=============================================");
    }
};

Stats stats;

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
}

void update_stats_according_to_domain_status_change(Stats& stats, const DomainStatusChange& domain_status_change)
{
    switch (domain_status_change)
    {
        case DomainStatusChange::new_ok:
            stats.domains_ok++;
            stats.sent_ok_notifications++;
            stats.sent_first_ok_notifications++;
            break;

        case DomainStatusChange::new_ko:
            stats.domains_ko++;
            stats.not_sent_first_ko_notifications++;
            break;

        case DomainStatusChange::ok_ok:
            stats.domains_ok++;
            stats.not_sent_still_ok_notifications++;
            break;

        case DomainStatusChange::ok_ko:
            stats.domains_ko++;
            stats.sent_ko_notifications++;
            break;

        case DomainStatusChange::ko_ok:
            stats.domains_ok++;
            stats.sent_ok_notifications++;
            break;

        case DomainStatusChange::ko_ko:
            stats.domains_ko++;
            stats.not_sent_still_ko_notifications++;
            break;

        case DomainStatusChange::ok_ok2:
            stats.domains_ok++;
            stats.sent_ok_notifications++;
            break;
    }
}

struct UnknownDomainStatusChange : std::exception
{
    const char* what() const throw ()
    {
        return "unknow domain status change";
    }
};

DomainStatusChange get_domain_status_change(
        const boost::optional<NotifiedDomainStatus>& _notified_domain_status,
        const DomainStatus& _domain_newest_status)
{
    if (!_notified_domain_status)
    {
        if (_domain_newest_status.status == DomainStatus::DomainStatusType::akm_status_candidate_ok)
        {
            return DomainStatusChange::new_ok;
        }
        else if (_domain_newest_status.status == DomainStatus::DomainStatusType::akm_status_candidate_ko)
        {
            return DomainStatusChange::new_ko;
        }
    }
    else {
        if (_notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_candidate_ok))
        {
            if (_domain_newest_status.status == DomainStatus::DomainStatusType::akm_status_candidate_ok)
            {
                return are_coherent(*_domain_newest_status.domain_state, *_notified_domain_status)
                        ? DomainStatusChange::ok_ok
                        : DomainStatusChange::ok_ok2;
            }
            else if (_domain_newest_status.status == DomainStatus::DomainStatusType::akm_status_candidate_ko)
            {
                return DomainStatusChange::ok_ko;
            }
        }
        else if ((_notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_candidate_ko)) ||
                (_notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::DomainStatusType::akm_status_managed_ok))) // fallen angel
        {
            if (_domain_newest_status.status == DomainStatus::DomainStatusType::akm_status_candidate_ok)
            {
                return DomainStatusChange::ko_ok;
            }
            else if (_domain_newest_status.status == DomainStatus::DomainStatusType::akm_status_candidate_ko)
            {
                return DomainStatusChange::ko_ko;
            }
        }
    }
    throw UnknownDomainStatusChange();
}

boost::optional<DomainStatus> lookup_intermediate_domain_status_to_notify(
        const std::pair<const Domain, DomainStatusStack::DomainStatuses>& domain, const NotifiedDomainStatus& _notified_domain_status)
{

    for (const auto& domain_status : boost::adaptors::reverse(domain.second))
    {
        if (domain_status.status != DomainStatus::DomainStatusType::akm_status_candidate_ok &&
                domain_status.scan_iteration.start_at > _notified_domain_status.last_at)
        {
            log()->debug("ok->...(ko)...->ok (ko_found@scan_iteration {})", to_string(domain_status.scan_iteration));
            return domain_status;
        }
    }
    return boost::optional<DomainStatus>();
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
    auto scan_result_rows =
            _storage.get_insecure_scan_result_rows_for_notify(
                    _minimal_scan_result_sequence_length_to_notify,
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

    for (const auto& scan_iteration : domain_state_stack.scan_iterations)
    {
        stats.domains_loaded += scan_iteration.second.size();
    }

    const int current_unix_time = _storage.get_current_unix_time();
    log()->debug("current unix time taken from db: {}", current_unix_time);

    DomainStatusStack domain_status_stack(domain_state_stack, _maximal_time_between_scan_results, current_unix_time);
    print(domain_status_stack);

    log()->debug(";== [command_notify] =========================================================================");

    stats.domains_loaded = domain_status_stack.domains_with_statuses.size();
    for (const auto& domain_with_statuses : domain_status_stack.domains_with_statuses)
    {
        const auto& domain = domain_with_statuses.first;
        const auto& domain_statuses = domain_with_statuses.second;

        try
        {
            if (domain_statuses.empty())
            {
                log()->error("no statuses for domain {}", domain.fqdn);
                stats.domains_unknown_no_data++;
                continue;
            }
            stats.domains_checked++;

            log()->debug("---");
            log()->debug("domain {}", to_string(domain));

            boost::optional<NotifiedDomainStatus> notified_domain_status =
                    _storage.get_last_notified_domain_status(domain.id);
            log()->debug("last notified status: {}",
                    notified_domain_status ? to_string(*notified_domain_status) : "NOT FOUND");

            DomainStatus domain_newest_status = domain_statuses.back();
            log()->debug("newest domain_status: {}", to_string(domain_newest_status));


            DomainStatusChange domain_status_change =
                    get_domain_status_change(notified_domain_status, domain_newest_status);
            log()->debug(to_string(domain_status_change));

            if (domain_status_change == DomainStatusChange::ok_ok) // oh, special case
            {
                const boost::optional<DomainStatus> intermediate_domain_status_to_notify =
                        lookup_intermediate_domain_status_to_notify(domain_with_statuses, *notified_domain_status);

                if (intermediate_domain_status_to_notify)
                {
                    stats.sent_first_ko_notifications++;

                    NotifiedDomainStatus new_notified_domain_status =
                            NotifiedDomainStatus(
                                    domain,
                                    *intermediate_domain_status_to_notify);
                    notify_and_save_domain_status(
                            new_notified_domain_status,
                            _storage,
                            _akm_backend,
                            _mailer_backend,
                            _dry_run,
                            _fake_contact_emails);

                    notified_domain_status = new_notified_domain_status;
                    log()->debug("last notified status now: {}",
                            notified_domain_status ? to_string(*notified_domain_status) : "NOT FOUND");

                    domain_status_change = get_domain_status_change(notified_domain_status, domain_newest_status);
                    log()->debug("now: " + to_string(domain_status_change));
                }
            }

            const bool domain_status_has_changed =
                    domain_status_change == DomainStatusChange::new_ok ||
                    domain_status_change == DomainStatusChange::ok_ko ||
                    domain_status_change == DomainStatusChange::ko_ok ||
                    domain_status_change == DomainStatusChange::ok_ok2;

            const bool notification_should_be_sent = domain_status_has_changed;

            if (notification_should_be_sent)
            {
                notify_and_save_domain_status(
                        NotifiedDomainStatus(
                                domain,
                                domain_newest_status),
                        _storage,
                        _akm_backend,
                        _mailer_backend,
                        _dry_run,
                        _fake_contact_emails);
            }

            update_stats_according_to_domain_status_change(stats, domain_status_change);

        }
        catch (const UnknownDomainStatusChange&)
        {
            log()->error("unknow domain status chage for domain: {} ", domain.fqdn);
        }
        catch (const NotificationFailed&)
        {
            log()->error("notification failed for domain domain: {} ", domain.fqdn);
            stats.sending_notification_failures++;
        }
    }

    stats.print();
}


} // namespace Fred::Akm
} // namespace Fred
