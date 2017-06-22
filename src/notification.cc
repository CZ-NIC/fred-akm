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

#include "src/notification.hh"

#include "src/enum_conversions.hh"
#include "src/log.hh"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

namespace Fred {
namespace Akm {

void notify_and_save_domain_status(
        const NotifiedDomainStatus& _notified_domain_status,
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const bool _dry_run)
{
    const std::string mail_from = "";
    const std::string mail_reply_to = "";

    const std::string template_name_str = Conversion::Enums::to_template_name(Conversion::Enums::to_notification_type(_notified_domain_status.domain_status));

    log()->info("shall notify template \"{}\"", template_name_str);
    log()->info("asking backend for emails for domain id {}", _notified_domain_status.domain.id);
    try
    {
        std::vector<std::string> tech_contacts = { "john.doe@example.com" };
        if (!_dry_run) {
            tech_contacts = _akm_backend.get_nsset_notification_emails_by_domain_id(_notified_domain_status.domain.id);
        }

        std::string emails = boost::algorithm::join(tech_contacts, ", ");
        Fred::Akm::IMailer::Header header(emails, mail_from, mail_reply_to);
        log()->debug("will send to email(s): {}", header.to);
        log()->debug("prepare template parameters for template \"{}\"", template_name_str);

        const IMailer::TemplateName template_name = template_name_str;

        IMailer::TemplateParameters template_parameters;
        template_parameters["domain"] = _notified_domain_status.domain.fqdn;
        template_parameters["zone"] = ".cz"; // TODO hardwired, get from domain.name
        template_parameters["datetime"] = _notified_domain_status.last_at;
        template_parameters["days_to_left"] = "7"; // TODO hardwired, get from config (notify_update_within_x_days)
        std::vector<std::string> keys;
        boost::split(keys, _notified_domain_status.serialized_cdnskeys, boost::is_any_of("|"));
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
            log()->debug("sending notification of state \"{}\"", to_string(_notified_domain_status));
            _storage.set_notified_domain_status(_notified_domain_status);
        }
    }
    catch (std::runtime_error& e)
    {
        log()->error(e.what());
        throw NotificationFailed();
    }
}

} // namespace Fred::Akm
} // namespace Fred
