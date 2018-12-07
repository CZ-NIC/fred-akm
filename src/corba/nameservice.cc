/*
 * Copyright (C) 2017  CZ.NIC, z. s. p. o.
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
#include <vector>
#include <boost/regex.hpp>

#include "src/corba/nameservice.hh"

namespace Fred {
namespace Akm {
namespace Corba {


std::vector<std::string> parse_ns_path(const std::string& _object_path)
{
    boost::regex split_on("[.]");
    boost::sregex_token_iterator beg(_object_path.begin(), _object_path.end(), split_on, -1);
    boost::sregex_token_iterator end;
    return {beg, end};
}


Nameservice::Nameservice(CORBA::ORB_ptr _orb, const std::string& _host, unsigned int _port)
    : orb_(_orb),
      host_(_host),
      port_(_port)
{
}


CORBA::Object_ptr Nameservice::resolve(const std::string& _object_path) const
{
    try
    {
        auto corbaname = "corbaname::" + host_ + ":" + std::to_string(port_);
        CORBA::Object_var ns = orb_->string_to_object(corbaname.c_str());
        CosNaming::NamingContext_var ns_root_ctx = CosNaming::NamingContext::_narrow(ns);
        if (CORBA::is_nil(ns_root_ctx))
        {
            throw std::runtime_error("couldn't connect to corba nameservice");
        }

        auto parsed_ns_path = parse_ns_path(_object_path);

        CosNaming::Name ctx_name;
        ctx_name.length(parsed_ns_path.size());
        for (std::vector<std::string>::size_type i = 0; i < parsed_ns_path.size() - 1; i++)
        {
            ctx_name[i].id = parsed_ns_path[i].c_str();
            ctx_name[i].kind = "context";
        }
        std::vector<std::string>::size_type obj_idx = parsed_ns_path.size() - 1;
        ctx_name[obj_idx].id = parsed_ns_path[obj_idx].c_str();
        ctx_name[obj_idx].kind = "Object";

        return ns_root_ctx->resolve(ctx_name);
    }
    catch (const CORBA::SystemException &e)
    {
        throw std::runtime_error(e._name());
    }
    catch (const CORBA::Exception &e)
    {
        throw std::runtime_error(e._name());
    }
}


} // namespace Fred
} // namespace Akm
} // namespace Corba


