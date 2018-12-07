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
#ifndef CORBA_CONTEXT_HH_F0575D63F39590879FA80C010411F66A//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_CONTEXT_HH_F0575D63F39590879FA80C010411F66A

#include <memory>
#include <string>

#include "src/corba/nameservice.hh"

namespace Fred {
namespace Akm {
namespace Corba {


class CorbaContext
{
public:
    CorbaContext(int _argc, char* _argv[], const std::string& _ns_host, const unsigned int _ns_port, const char* options[][2]=0);

    const Nameservice& get_nameservice() const;

private:
    CORBA::ORB_var orb_;
    Nameservice ns_;
};


} // namespace Fred
} // namespace Akm
} // namespace Corba

#endif//CORBA_CONTEXT_HH_F0575D63F39590879FA80C010411F66A
