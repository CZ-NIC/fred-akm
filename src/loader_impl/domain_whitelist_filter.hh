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
#ifndef DOMAIN_WHITELIST_FILTER_HH_2A62B8576AF23D16826FD873281A6AFC//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define DOMAIN_WHITELIST_FILTER_HH_2A62B8576AF23D16826FD873281A6AFC

#include "src/i_loader_filter.hh"

#include <string>
#include <unordered_set>

namespace Fred {
namespace Akm {


class DomainWhitelistFilter : public ILoaderOutputFilter
{
public:
    DomainWhitelistFilter(std::string _filename);

    void apply(DomainScanTaskCollection& _collection) const;

private:
    std::unordered_set<std::string> domain_whitelist_;
};


} //namespace Akm
} //namespace Fred



#endif//DOMAIN_WHITELIST_FILTER_HH_2A62B8576AF23D16826FD873281A6AFC
