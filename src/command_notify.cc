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

#include "src/nameserver_domains.hh"
#include "src/domain_state_stack.hh"
#include "src/domain_state.hh"
#include "src/notified_domain_state.hh"
#include "src/log.hh"
#include "src/utils.hh"

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <chrono>
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

    int sent_notifications;
    int sent_ok_notifications;
    int sent_ko_notifications;
    int sent_first_ok_notifications;
    int sent_first_ko_notifications;
    int not_sent_first_ko_notifications;
    int not_sent_still_ok_notifications;
    int not_sent_still_ko_notifications;

    void print()
    {
        log()->info("================== STATS ====================");
        log()->info("domains loaded:                      {:>8}", domains_loaded);
        log()->info("domains checked:                     {:>8}", domains_checked);
        log()->info("  domains ok:                        {:>8}", domains_ok);
        log()->info("  domains ko:                        {:>8}", domains_ko);
        log()->info("---------------------------------------------");
        log()->info("sent notifications:                  {:>8}", sent_notifications);
        log()->info("  sent ok notifications:             {:>8}", sent_ok_notifications);
        log()->info("    sent first ok notifications:     {:>8}", sent_first_ok_notifications);
        log()->info("  sent ko notifications:             {:>8}", sent_ko_notifications);
        log()->info("    sent first ko notifications:     {:>8}", sent_first_ko_notifications);
        log()->info("not sent first ko notifications:     {:>8}", not_sent_first_ko_notifications);
        log()->info("not sent still ok notifications:     {:>8}", not_sent_still_ok_notifications);
        log()->info("not sent still ko notifications:     {:>8}", not_sent_still_ko_notifications);
        log()->info("=============================================");
    }
};

Stats stats;

void send_and_save_notified_domain_state(
        const NotifiedDomainState& _notified_domain_state,
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const std::string& _template_name,
        const bool _dry_run)
{
    const std::string mail_from = "";
    const std::string mail_reply_to = "";

    log()->info("shall notify template \"{}\"", _template_name);
    log()->info("asking backend for emails for domain id {}", _notified_domain_state.domain.id);
    try
    {
        auto tech_contacts = _akm_backend.get_nsset_notification_emails_by_domain_id(_notified_domain_state.domain.id);

        std::string emails = boost::algorithm::join(tech_contacts, ", ");
        Fred::Akm::IMailer::Header header(emails, mail_from, mail_reply_to);
        log()->debug("will send to email(s): {}", header.to);
        log()->debug("prepare template parameters for template \"{}\"", _template_name);

        const IMailer::TemplateName template_name = _template_name;

        IMailer::TemplateParameters template_parameters;
        template_parameters["domain"] = _notified_domain_state.domain.fqdn;
        template_parameters["zone"] = ".cz"; // TODO hardwired, get from domain.name
        template_parameters["datetime"] = _notified_domain_state.last_at;
        template_parameters["days_to_left"] = "7"; // TODO hardwired, get from config (notify_update_within_x_days)
        std::vector<std::string> keys;
        boost::split(keys, _notified_domain_state.serialized_cdnskeys, boost::is_any_of("|"));
        for (int i = 0; i < keys.size(); ++i) {
            template_parameters["keys." + std::to_string(i)] = keys[i];
        }

        for (const auto& template_parameter : template_parameters)
        {
            log()->debug("template_parameter[\"{}\"] = \"{}\"", template_parameter.first, template_parameter.second);
        }

        if (!_dry_run) {
            log()->debug("sending notification to template_name \"{}\"", template_name);
            _mailer_backend.enqueue(header, template_name, template_parameters);
            // TODO (exceptions thrown by enqueue? (combination of "email sent + exception throw" would spam)
            _storage.set_notified_domain_state(_notified_domain_state);
        }
    }
    catch (std::runtime_error& e)
    {
        // TODO log error and continue
        stats.print();
        throw e;
    }
}

bool are_coherent(const DomainState& _domain_state, const NotifiedDomainState& _notified_domain_state)
{
    return
            _domain_state.domain == _notified_domain_state.domain &&
            serialize(_domain_state.cdnskeys) == _notified_domain_state.serialized_cdnskeys;
}

} // namespace Fred::Akm::{anonymous}

void command_notify(
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        unsigned long _maximal_time_between_scan_results,
        unsigned long _minimal_scan_result_sequence_length_to_notify,
        bool  _notify_from_last_iteration_only,
        const bool _dry_run)
{
    // TODO enum
    const int domain_status_ok = 0;
    const int domain_status_ko = 1;
    std::map<int, std::string> template_names = {
        {domain_status_ok, "akm_candidate_state_ok"},
        {domain_status_ko, "akm_candidate_state_ko"}
    };

    auto scan_result_rows =
            _storage.get_insecure_scan_result_rows(
                    _minimal_scan_result_sequence_length_to_notify,
                    _notify_from_last_iteration_only);

    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    DomainStateStack haystack(scan_result_rows);

    print(haystack);

    stats.domains_loaded = haystack.domains.size();
    for (const auto& domain : haystack.domains) {
        log()->info(domain.first.fqdn);
        boost::optional<DomainState> newest_domain_state =
                get_last_domain_state_if_domain_nameservers_are_coherent(
                        domain.first,
                        domain.second,
                        _maximal_time_between_scan_results,
                        0);

        const bool is_newest_state_with_deletekey = newest_domain_state && has_deletekey(*newest_domain_state);

        if (is_newest_state_with_deletekey) {
            log()->info("domain state seems ok, but DELETE KEY(S) are present -> status is KO");
        }

        const int domain_status =
                newest_domain_state && !is_newest_state_with_deletekey
                        ? domain_status_ok
                        : domain_status_ko;

        log()->debug("newest domain_status: {}: {}",
                domain_status == domain_status_ok ? "OK" : "KO",
                to_string(newest_domain_state.value_or(DomainState())));

        boost::optional<NotifiedDomainState> notified_domain_state =
                _storage.get_last_notified_domain_state(domain.first.id);

        log()->debug("last notified state: {}: {}",
                !notified_domain_state
                        ? "NOT FOUND"
                        : notified_domain_state->notification_type == domain_status_ok
                                ? "OK"
                                : notified_domain_state->notification_type == domain_status_ko
                                        ? "KO"
                                        : "UNKNOWN NOTIFICATION TYPE",
                to_string(notified_domain_state.value_or(NotifiedDomainState())));

        if (domain_status == domain_status_ok)
        {
            if (notified_domain_state && (notified_domain_state->notification_type == domain_status_ok))
            {
                stats.domains_ok++;
                if (are_coherent(*newest_domain_state, *notified_domain_state))
                {
                    log()->debug("domain status ok, already notified");
                    stats.not_sent_still_ok_notifications++;
                    continue;
                }
                else
                {
                    log()->debug("ok status recently notified, but for different domain state -> notify the new one now");
                    stats.sent_notifications++;
                    stats.sent_ok_notifications++;
                }
            }
            if (!notified_domain_state)
            {
                stats.sent_first_ok_notifications++;
            }

            log()->debug("domain status: OK");

            send_and_save_notified_domain_state(
                    NotifiedDomainState(
                            newest_domain_state->domain,
                            serialize(newest_domain_state->cdnskeys), // boost::algorithm::join(newest_domain_state->cdnskeys | boost::adaptors::map_keys, ","),
                            domain_status,
                            newest_domain_state->scan_at),
                    _storage,
                    _akm_backend,
                    _mailer_backend,
                    template_names[domain_status],
                    _dry_run);
        }
        else {
            stats.domains_ko++;
            if (!notified_domain_state) {
                log()->debug("domain status KO, but OK STATUS NEVER NOTIFIED (so this is NOT THE OK->KO STATUS CHANGE, bye for now)");
                stats.not_sent_first_ko_notifications++;
                continue;
            }
            if (notified_domain_state->notification_type == domain_status_ok) {
                log()->debug("domain status KO, last notified status OK, so NOTIFY OK-KO CHANGE now");
                stats.sent_first_ko_notifications++;

                send_and_save_notified_domain_state(
                        NotifiedDomainState(
                                newest_domain_state->domain,
                                "",
                                domain_status,
                                newest_domain_state->scan_at),
                        _storage,
                        _akm_backend,
                        _mailer_backend,
                        template_names[domain_status],
                        _dry_run);
            }
            else if (notified_domain_state->notification_type == domain_status_ko) {
                stats.not_sent_still_ko_notifications++;
                log()->debug("domain ko, already notified");
                continue;
            }
            else {
                throw std::runtime_error("unknown notification_type");
            }
        }
        stats.domains_checked++;
    }

    stats.print();
    log()->debug("command notify done");
}


} //namespace Fred::Akm
} //namespace Fred
