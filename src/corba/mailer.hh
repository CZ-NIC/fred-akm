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
#ifndef CORBA_MAILER_HH_207838907A1B10CB5C28A8E6721AFAD7//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_MAILER_HH_207838907A1B10CB5C28A8E6721AFAD7

#include "src/corba/nameservice.hh"
#include "src/i_mailer.hh"

namespace Fred {
namespace Akm {
namespace Corba {


class Mailer : public IMailer
{
public:
    Mailer(const Nameservice& _ns, const std::string& _ns_path_mailer);

    void enqueue(
        const Header& _header,
        const TemplateName& _template_name,
        const TemplateParameters& _template_params
    ) const;

private:
    const Nameservice& ns_;
    std::string ns_path_mailer_;
};


} // namespace Fred::Akm::Corba
} // namespace Fred::Akm
} // namespace Fred

#endif//CORBA_MAILER_HH_207838907A1B10CB5C28A8E6721AFAD7
