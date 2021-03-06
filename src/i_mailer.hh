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
#ifndef I_MAILER_HH_6D59404C3B72544DBC5990EF3624B613//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define I_MAILER_HH_6D59404C3B72544DBC5990EF3624B613

#include <string>
#include <map>
#include <boost/optional.hpp>

#include "src/corba/generated/Mailer.hh"

namespace Fred {
namespace Akm {


class IMailer
{
public:
    typedef std::string TemplateName;
    typedef std::map<std::string, std::string> TemplateParameters;

    struct Header
    {
        Header(const std::string& _to,
               const boost::optional<std::string>& _from = boost::optional<std::string>(),
               const boost::optional<std::string>& _reply_to = boost::optional<std::string>())
            : to(_to), from(_from), reply_to(_reply_to)
        {
        }

        std::string to;
        boost::optional<std::string> from;
        boost::optional<std::string> reply_to;
    };

    virtual void enqueue(
        const Header& _header,
        const TemplateName& _template_name,
        const TemplateParameters& _template_params
    ) const = 0;
};


} // namespace Fred
} // namespace Akm

#endif//I_MAILER_HH_6D59404C3B72544DBC5990EF3624B613
