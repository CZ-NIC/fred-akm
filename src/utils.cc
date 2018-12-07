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
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>

#include "src/utils.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


void split_on(const std::string& _in_string, const char _delimiter, std::vector<std::string>& _out_tokens)
{
    auto beg_ptr = _in_string.begin();
    auto end_ptr = _in_string.end();
    auto nxt_ptr = std::find(beg_ptr, end_ptr, _delimiter);
    while (nxt_ptr != end_ptr)
    {
        _out_tokens.emplace_back(std::string(beg_ptr, nxt_ptr));
        beg_ptr = nxt_ptr + 1;
        nxt_ptr = std::find(beg_ptr, end_ptr, _delimiter);
    }
    _out_tokens.emplace_back(beg_ptr, nxt_ptr);
}


std::unordered_map<std::string, std::string> variable_map_to_string_map(
    const boost::program_options::variables_map& _map
)
{
    std::unordered_map<std::string, std::string> str_map;

    for (const auto kv : _map)
    {
        const std::string& name = kv.first;
        std::string value = "";
        std::string flags = "";

        if ((kv.second.value()).empty())
        {
            flags.append("[empty]");
        }
        if (kv.second.defaulted())
        {
            flags.append("[default]");
        }

        bool found = false;
        try
        {
            value = boost::lexical_cast<std::string>(boost::any_cast<int>(kv.second.value()));
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            value = boost::lexical_cast<std::string>(boost::any_cast<unsigned int>(kv.second.value()));
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            value = boost::lexical_cast<std::string>(boost::any_cast<long>(kv.second.value()));
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            value = boost::lexical_cast<std::string>(boost::any_cast<unsigned long>(kv.second.value()));
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            bool aux = boost::any_cast<bool>(kv.second.value());
            if (aux)
            {
                value = "on";
            }
            else
            {
                value = "off";
            }
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            value = boost::lexical_cast<std::string>(boost::any_cast<double>(kv.second.value()));
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            value = std::string(boost::any_cast<const char *>(kv.second.value()));
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            value = boost::any_cast<std::string>(kv.second.value());
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        try
        {
            auto aux = boost::any_cast<std::vector<std::string>>(kv.second.value());
            value = "[ " + boost::algorithm::join(aux, ", ") + " ]";
            found = true;
        }
        catch (const boost::bad_any_cast &) { }
        if (!found)
        {
            value = "<unhandled type>";
        }

        str_map[name] = value + (!flags.empty() ? " " + flags : "");
    }
    return str_map;
}


} // namespace Akm
} // namespace Fred
