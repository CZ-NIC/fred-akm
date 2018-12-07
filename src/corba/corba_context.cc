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
#include "src/corba/corba_context.hh"

namespace Fred {
namespace Akm {
namespace Corba {


CorbaContext::CorbaContext(int _argc, char* _argv[], const std::string& _ns_host, const unsigned int _ns_port, const char* options[][2])
    : orb_(CORBA::ORB_init(_argc, _argv, "", options)),
      ns_(orb_, _ns_host, _ns_port)
{
}


const Nameservice& CorbaContext::get_nameservice() const
{
    return ns_;
}


} // namespace Fred
} // namespace Akm
} // namespace Corba



