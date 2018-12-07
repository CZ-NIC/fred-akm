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
#ifndef KEYSET_HH_7AC3F151D2294A44A5DF4DFCB9A76D3B
#define KEYSET_HH_7AC3F151D2294A44A5DF4DFCB9A76D3B

#include "src/dnskey.hh"

#include <string>
#include <vector>

namespace Fred {
namespace Akm {


struct Keyset
{
    Keyset()
        : dnskeys()
    {
    }

    Keyset(const std::vector<Dnskey>& _dnskeys)
        : dnskeys(_dnskeys)
    {
    }

    std::vector<Dnskey> dnskeys;
};

std::string to_string(const Keyset& _keyset);

} // namespace Fred
} // namespace Akm

#endif
