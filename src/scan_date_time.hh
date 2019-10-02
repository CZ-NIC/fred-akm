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
#ifndef SCAN_DATE_TIME_HH_16772CB07EAB427090EBC895344BA2D9
#define SCAN_DATE_TIME_HH_16772CB07EAB427090EBC895344BA2D9

#include <string>

struct ScanDateTime
{
    ScanDateTime()
        : scan_date_time(),
          scan_seconds()
    {
    }

    ScanDateTime(
            const std::string& _scan_date_time,
            const int _scan_seconds)
        : scan_date_time(_scan_date_time),
          scan_seconds(_scan_seconds)
    {
    }

    bool empty() const
    {
        return scan_date_time.empty();
    }

    bool operator==(const ScanDateTime& _rhs) const
    {
        return scan_seconds == _rhs.scan_seconds;
    }

    bool operator<(const ScanDateTime& _rhs) const
    {
        return scan_seconds < _rhs.scan_seconds;
    }

    bool operator>(const ScanDateTime& _rhs) const
    {
        return scan_seconds > _rhs.scan_seconds;
    }

    std::string scan_date_time;
    long long scan_seconds;
};

std::string to_string(const ScanDateTime& scan_date_time);
std::string to_db_string(const ScanDateTime& scan_date_time);
std::string to_template_string(const ScanDateTime& scan_date_time);

#endif
