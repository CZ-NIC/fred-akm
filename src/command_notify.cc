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
    log()->info("asking backend for emails for domain id {}", _notified_domain_state.domain_id);
    auto tech_contacts = _akm_backend.get_nsset_notification_emails_by_domain_id(_notified_domain_state.domain_id);

    std::string emails = boost::algorithm::join(tech_contacts, ", ");
    Fred::Akm::IMailer::Header header(emails, mail_from, mail_reply_to);
    log()->debug("will send to email(s): {}", header.to);
    log()->debug("prepare template parameters for template \"{}\"", _template_name);

    const IMailer::TemplateName template_name = _template_name;

    IMailer::TemplateParameters template_parameters;
    template_parameters["domain"] = _notified_domain_state.domain_name; // TODO hardcoded
    template_parameters["zone"] = ".cz"; // TODO hardcoded
    template_parameters["datetime"] = _notified_domain_state.last_at;
    template_parameters["days_to_left"] = "7"; // TODO get from config (notify_update_within_x_days)
    std::vector<std::string> keys;
    boost::split(keys, _notified_domain_state.serialized_cdnskeys, boost::is_any_of("|"));
    for (int i = 0; i < keys.size(); ++i) {
        template_parameters["keys." + std::to_string(i)] = keys[i];
    }

    for (const auto& template_parameter : template_parameters)
    {
        log()->debug("template_parameter[\"{}\"] = \"{}\"", template_parameter.first, template_parameter.second);
    }

    try
    {
        if (!_dry_run) {
            log()->debug("sending notification to template_name \"{}\"", template_name);
            _mailer_backend.enqueue(header, template_name, template_parameters);
            // TODO (exceptions thrown by enqueue? (combination of "email sent + exception throw" would spam)
            _storage.set_notified_domain_state(_notified_domain_state);
        }
    }
    catch (std::runtime_error& e)
    {
        // TODO log error
        throw e;
    }
}

bool are_coherent(const DomainState& _domain_state, const NotifiedDomainState& _notified_domain_state)
{
    return
            _domain_state.domain_id == _notified_domain_state.domain_id &&
            _domain_state.domain_name == _notified_domain_state.domain_name &&
            _domain_state.has_keyset == _notified_domain_state.has_keyset &&
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
    const int domain_state_ok = 0;
    const int domain_state_ko = 1;
    std::map<int, std::string> template_names = {
        {domain_state_ok, "akm_candidate_state_ok"},
        {domain_state_ko, "akm_candidate_state_ko"}
    };

    auto scan_result_rows = _storage.get_insecure_scan_result_rows(_minimal_scan_result_sequence_length_to_notify, _notify_from_last_iteration_only); // FIXME
    log()->debug("got from database {} scan result(s)", scan_result_rows.size());

    DomainStateStack haystack(scan_result_rows);

    print(haystack);

    for (const auto& domain : haystack.domains) {
        bool domain_nameservers_coherent;
        log()->info(domain.first.fqdn);
        boost::optional<DomainState> newest_domain_state =
                get_last_domain_state(
                        domain.first,
                        domain.second,
                        _maximal_time_between_scan_results,
                        0,
                        domain_nameservers_coherent);

        log()->debug("newest domain_state: {}: {}", domain_nameservers_coherent ? "OK" : "KO", to_string(newest_domain_state.value_or(DomainState())));

        boost::optional<NotifiedDomainState> notified_domain_state = _storage.get_last_notified_domain_state(domain.first.id);

        log()->debug("last notified state: {}: {}", !notified_domain_state ? "NOT FOUND" : notified_domain_state->notification_type == domain_state_ok ? "OK" : notified_domain_state->notification_type == domain_state_ko ? "KO" : "UNKNOWN NOTIFICATION TYPE", to_string(notified_domain_state.value_or(NotifiedDomainState())));

        if (domain_nameservers_coherent)
        {
            if (notified_domain_state && (notified_domain_state->notification_type == domain_state_ok))
            {
                if (are_coherent(*newest_domain_state, notified_domain_state.value_or(NotifiedDomainState())))
                {
                    log()->debug("domain ok, already notified");
                    continue;
                }
                else
                {
                    log()->debug("ok state notified, but different one, notify the new one");
                }
            }

            log()->debug("domain ok");

            const int domain_state = domain_state_ok;
            send_and_save_notified_domain_state(
                    NotifiedDomainState(
                            newest_domain_state->domain_id,
                            newest_domain_state->domain_name,
                            newest_domain_state->has_keyset,
                            serialize(newest_domain_state->cdnskeys), // boost::algorithm::join(newest_domain_state->cdnskeys | boost::adaptors::map_keys, ","),
                            domain_state,
                            newest_domain_state->scan_at),
                    _storage,
                    _akm_backend,
                    _mailer_backend,
                    template_names[domain_state],
                    _dry_run);
        }
        else {
            if (!notified_domain_state) {
                log()->debug("domain ko, but ok state never notified (so this is not the ok->ko state change, bye)");
                continue;
            }
            if (notified_domain_state->notification_type == domain_state_ok) {
                log()->debug("domain ko, last notified state ok, so notify ok-ko change");

                const int domain_state = domain_state_ko;
                send_and_save_notified_domain_state(
                        NotifiedDomainState(
                                newest_domain_state->domain_id,
                                newest_domain_state->domain_name,
                                newest_domain_state->has_keyset,
                                "", //serialize(newest_domain_state->cdnskeys), // boost::algorithm::join(newest_domain_state->cdnskeys | boost::adaptors::map_keys, ","),
                                domain_state,
                                newest_domain_state->scan_at),
                        _storage,
                        _akm_backend,
                        _mailer_backend,
                        template_names[domain_state],
                        _dry_run);
            }
            else if (notified_domain_state->notification_type == domain_state_ko) {
                log()->debug("domain ko, already notified");
                continue;
            }
            else {
                throw std::runtime_error("unknown notification_type");
            }
        }
    }

    log()->debug("done");
}


} //namespace Fred::Akm
} //namespace Fred
