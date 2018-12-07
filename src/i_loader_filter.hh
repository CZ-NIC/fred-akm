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
#ifndef I_LOADER_OUTPUT_FILTER_HH_91740628434C3CA25C9FD101A19C777D//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_LOADER_OUTPUT_FILTER_HH_91740628434C3CA25C9FD101A19C777D

#include "src/i_loader.hh"
#include <vector>


namespace Fred {
namespace Akm {


class ILoaderOutputFilter
{
public:
    virtual ~ILoaderOutputFilter() = default;
    virtual void apply(DomainScanTaskCollection& _collection) const = 0;
};


} //namespace Akm
} //namespace Fred


#endif//I_LOADER_OUTPUT_FILTER_HH_91740628434C3CA25C9FD101A19C777D
