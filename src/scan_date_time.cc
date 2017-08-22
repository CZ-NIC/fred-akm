/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "scan_date_time.hh"

#include <string>

std::string to_string(const ScanDateTime& scan_date_time)
{
    return scan_date_time.scan_date_time;
}

std::string to_db_string(const ScanDateTime& scan_date_time)
{
    return scan_date_time.scan_date_time;
}

std::string to_template_string(const ScanDateTime& scan_date_time)
{
    return scan_date_time.scan_date_time; // TODO localize?
}
