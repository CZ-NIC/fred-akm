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
#ifndef CORBA_NAMESERVICE_D305B935F627D70603120E224F16D731//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_NAMESERVICE_D305B935F627D70603120E224F16D731

#include <string>
#include <omniORB4/CORBA.h>

namespace Fred {
namespace Akm {
namespace Corba {


class Nameservice
{
public:
    Nameservice(CORBA::ORB_ptr _orb, const std::string &_host, unsigned int _port);

    CORBA::Object_ptr resolve(const std::string& _object_path) const;

private:
    CORBA::ORB_ptr orb_;
    std::string host_;
    unsigned int port_;
};


} // namespace Fred
} // namespace Akm
} // namespace Corba

#endif//CORBA_NAMESERVICE_D305B935F627D70603120E224F16D731
