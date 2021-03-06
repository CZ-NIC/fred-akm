/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
#include "src/notification.hh"

#include "src/enum_conversions.hh"
#include "src/log.hh"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

namespace Fred {
namespace Akm {

void save_domain_status(
        const DomainNotifiedStatus& _domain_notified_status,
        const IStorage& _storage,
        const bool _dry_run)
{
    if (!_dry_run)
    {
        log()->debug("saving domain_status_notification of state \"{}\"", to_string(_domain_notified_status));
        _storage.set_domain_notified_status(_domain_notified_status);
    }
}

void notify_and_save_domain_status(
        const DomainNotifiedStatus& _domain_notified_status,
        const IStorage& _storage,
        const IAkm& _akm_backend,
        const IMailer& _mailer_backend,
        const bool _dry_run,
        const bool _fake_contact_emails)
{
    const std::string mail_from = "";
    const std::string mail_reply_to = "";

    const std::string template_name_str = Conversion::Enums::to_template_name(Conversion::Enums::to_notification_type(_domain_notified_status.domain_status));

    log()->info("shall notify template \"{}\" to domain {}", template_name_str, _domain_notified_status.domain.fqdn);
    log()->debug("asking backend for emails for domain id {}", _domain_notified_status.domain.id);
    try
    {
        std::vector<std::string> tech_contacts = { "fake.contact.email.akm@example.com" };
        const bool real_contact_emails = !(_dry_run && _fake_contact_emails);
        if (real_contact_emails)
        {
            tech_contacts = _akm_backend.get_email_addresses_by_domain_id(_domain_notified_status.domain.id);
        }

        std::string emails = boost::algorithm::join(tech_contacts, ", ");
        Fred::Akm::IMailer::Header header(emails, mail_from, mail_reply_to);
        log()->debug("will send to email(s): {}", header.to);
        log()->debug("prepare template parameters for template \"{}\"", template_name_str);

        const IMailer::TemplateName template_name = template_name_str;

        IMailer::TemplateParameters template_parameters;
        template_parameters["domain"] = _domain_notified_status.domain.fqdn;
        template_parameters["zone"] = ".cz"; // TODO hardwired, get from domain.name
        template_parameters["datetime"] = to_template_string(_domain_notified_status.last_at);
        template_parameters["days_to_left"] = "7"; // TODO hardwired, get from config (notify_update_within_x_days)
        std::vector<std::string> keys;
        boost::split(keys, _domain_notified_status.serialized_cdnskeys, boost::is_any_of("|"));
        for (size_t i = 0; i < keys.size(); ++i)
        {
            template_parameters["keys." + std::to_string(i)] = keys[i];
        }

        for (const auto& template_parameter : template_parameters)
        {
            log()->debug("template_parameter[\"{}\"] = \"{}\"", template_parameter.first, template_parameter.second);
        }

        if (!_dry_run)
        {
            log()->info("sending notification to template_name \"{}\"", template_name);
            _mailer_backend.enqueue(header, template_name, template_parameters);
            // TODO (exceptions thrown by enqueue? (combination of "email sent + exception throw" would spam)
            save_domain_status(_domain_notified_status, _storage, _dry_run);
        }
    }
    catch (const ObjectNotFound& e)
    {
        log()->error(e.what());
        throw NotificationNotPossible();
    }
    catch (const AkmException& e)
    {
        log()->error(e.what());
        throw NotificationFailed();
    }
    catch (std::runtime_error& e)
    {
        log()->error(e.what());
        throw NotificationFailed();
    }
}

} // namespace Fred::Akm
} // namespace Fred
