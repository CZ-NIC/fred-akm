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
#include "src/log.hh"
#include "src/nameserver_domains.hh"
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

    int sent_notifications;
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
        log()->info("================== STATS ====================");
        log()->info("domains loaded:                      {:>8}", domains_loaded);
        log()->info("  domains checked:                   {:>8}", domains_checked);
        log()->info("    domains ok:                      {:>8}", domains_ok);
        log()->info("    domains ko:                      {:>8}", domains_ko);
        log()->info("    no data:                         {:>8}", domains_unknown_no_data);
        log()->info("---------------------------------------------");
        log()->info("sent notifications:                  {:>8}", sent_notifications);
        log()->info("  sent ok notifications:             {:>8}", sent_ok_notifications);
        log()->info("    sent first ok notifications:     {:>8}", sent_first_ok_notifications);
        log()->info("  sent ko notifications:             {:>8}", sent_ko_notifications);
        log()->info("    sent first ko notifications:     {:>8}", sent_first_ko_notifications);
        log()->info("not sent first ko notifications:     {:>8}", not_sent_first_ko_notifications);
        log()->info("not sent still ok notifications:     {:>8}", not_sent_still_ok_notifications);
        log()->info("not sent still ko notifications:     {:>8}", not_sent_still_ko_notifications);
        log()->info("sending notification failures:       {:>8}", sending_notification_failures);
        log()->info("=============================================");
    }
};

Stats stats;

} // namespace Fred::Akm::{anonymous}

void command_notify(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        unsigned long _maximal_time_between_scan_results,
        unsigned long _minimal_scan_result_sequence_length_to_notify,
        bool  _notify_from_last_scan_iteration_only,
        const bool _dry_run)
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

    for (const auto& scan_iteration : domain_state_stack.scan_iterations) {
        stats.domains_loaded += scan_iteration.second.size();
    }

    DomainStatusStack domain_status_stack(domain_state_stack, _maximal_time_between_scan_results);
    print(domain_status_stack);

    log()->info(";== [command_notify] =========================================================================");

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

        if (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ok) && newest_domain_status.status == DomainStatus::akm_status_candidate_ok)
        {

            bool no_ok_notification_sent = false;
            for (const auto& domain_status : boost::adaptors::reverse(domain.second))
            {
                if (domain_status.status != DomainStatus::akm_status_candidate_ok &&
                    domain_status.scan_iteration.start_at > notified_domain_status->last_at)
                {
                    log()->debug("ok->...(ko)...->ok (ko@scan_iteration {})", to_string(domain_status.scan_iteration));
                    stats.sent_first_ok_notifications++;
                    no_ok_notification_sent = true;
                    try {
                        NotifiedDomainStatus new_notified_domain_status =
                                NotifiedDomainStatus(
                                        domain.first,
                                        domain_status);
                        notify_and_save_domain_status(
                                new_notified_domain_status,
                                _storage,
                                _akm_backend,
                                _mailer_backend,
                                _dry_run);

                        notified_domain_status = new_notified_domain_status;
                        log()->debug("last notified status now: {}: {}",
                                notified_domain_status ? to_status_string(*notified_domain_status) : "NOT FOUND",
                                notified_domain_status ? to_string(*notified_domain_status) : "");

                    }
                    catch (const NotificationFailed&)
                    {
                        stats.sending_notification_failures++;
                    }
                }
            }
            if (no_ok_notification_sent) {
                stats.sent_first_ko_notifications++;
            }
            else {
                stats.not_sent_still_ok_notifications++;
            }
        }

        if (!notified_domain_status && newest_domain_status.status == DomainStatus::akm_status_candidate_ok)
        {
            log()->debug("new->ok");
        }
        if (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ok) && newest_domain_status.status == DomainStatus::akm_status_candidate_ko)
        {
            log()->debug("ok->ko");
        }
        if (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ko) && newest_domain_status.status == DomainStatus::akm_status_candidate_ok)
        {
            log()->debug("ko->ok");
        }
        if (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ok) && newest_domain_status.status == DomainStatus::akm_status_candidate_ok && 
             !are_coherent(*newest_domain_status.domain_state, *notified_domain_status))
        {
            log()->debug("ok->ok2 (different)");
        }

        if ((!notified_domain_status && newest_domain_status.status == DomainStatus::akm_status_candidate_ok) ||
            (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ok) && newest_domain_status.status == DomainStatus::akm_status_candidate_ko) ||
            (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ko) && newest_domain_status.status == DomainStatus::akm_status_candidate_ok) ||
            (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ok) && newest_domain_status.status == DomainStatus::akm_status_candidate_ok && 
             !are_coherent(*newest_domain_status.domain_state, *notified_domain_status)) )
        {

            stats.sent_notifications++;
            if (newest_domain_status.status == DomainStatus::akm_status_candidate_ok) {
                stats.sent_ok_notifications++;
                stats.domains_ok++;
            }
            else if (newest_domain_status.status == DomainStatus::akm_status_candidate_ko) {
                stats.sent_ko_notifications++;
                stats.domains_ko++;
            }
            if (!notified_domain_status && newest_domain_status.status == DomainStatus::akm_status_candidate_ok)
            {
                stats.sent_first_ok_notifications++;
            }
            try {
                notify_and_save_domain_status(
                        NotifiedDomainStatus(
                                domain.first,
                                newest_domain_status),
                        _storage,
                        _akm_backend,
                        _mailer_backend,
                        _dry_run);
            }
            catch (const NotificationFailed&)
            {
                stats.sending_notification_failures++;
            }
        }
        if (notified_domain_status && notified_domain_status->notification_type == Conversion::Enums::to_notification_type(DomainStatus::akm_status_candidate_ko) && newest_domain_status.status == DomainStatus::akm_status_candidate_ko)
        {
            stats.not_sent_still_ko_notifications++;
            // log()->info("DO NOT NOTIFY...");
        }

    }

    stats.print();
    log()->debug("command notify done");
}


} // namespace Fred::Akm
} // namespace Fred
