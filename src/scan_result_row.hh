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

#ifndef SCAN_RESULT_ROW_HH_598C167E29DF4307BB188D70F565A42A
#define SCAN_RESULT_ROW_HH_598C167E29DF4307BB188D70F565A42A

#include "src/cdnskey.hh"
#include "src/domain_state.hh"
#include "src/scan_type.hh"
#include "src/scan_date_time.hh"

#include <ostream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {

struct ScanResultRow
{
    ScanResultRow()
        : id(),
          scan_iteration_id(),
          scan_at(),
          domain_id(),
          domain_name(),
          scan_type(),
          nameserver(),
          nameserver_ip(),
          cdnskey()
    {
    }

    ScanResultRow(
            long long _id,
            long long _scan_iteration_id,
            const ScanDateTime& _scan_at,
            long long _domain_id,
            const std::string& _domain_name,
            ScanType _scan_type,
            const std::string& _nameserver,
            const std::string& _nameserver_ip,
            const Cdnskey& _cdnskey)
        : id(_id),
          scan_iteration_id(_scan_iteration_id),
          scan_at(_scan_at),
          domain_id(_domain_id),
          domain_name(_domain_name),
          scan_type(_scan_type),
          nameserver(_nameserver),
          nameserver_ip(_nameserver_ip),
          cdnskey(_cdnskey)
    {
    }

    ScanResultRow(const ScanResultRow& _scan_result_row)
        : id(_scan_result_row.id),
          scan_iteration_id(_scan_result_row.scan_iteration_id),
          scan_at(_scan_result_row.scan_at),
          domain_id(_scan_result_row.domain_id),
          domain_name(_scan_result_row.domain_name),
          scan_type(_scan_result_row.scan_type),
          nameserver(_scan_result_row.nameserver),
          nameserver_ip(_scan_result_row.nameserver_ip),
          cdnskey(_scan_result_row.cdnskey)
    {
    }

    long long id;
    long long scan_iteration_id;
    ScanDateTime scan_at;
    long long domain_id;
    std::string domain_name;
    ScanType scan_type;
    std::string nameserver;
    std::string nameserver_ip;
    Cdnskey cdnskey;
};

typedef std::vector<ScanResultRow> ScanResultRows;

std::ostream& operator<<(std::ostream& os, const ScanResultRow& scan_result_row);
std::string to_string(const ScanResultRow& scan_result_row);

bool is_valid(const ScanResultRow& _scan_result_row);
bool is_insecure(const ScanResultRow& _scan_result_row);
bool is_insecure_with_data(const ScanResultRow& _scan_result_row);
bool is_secure_auto(const ScanResultRow& _scan_result_row);
bool is_secure_noauto(const ScanResultRow& _scan_result_row);
bool is_secure_auto_with_data(const ScanResultRow& _scan_result_row);
bool is_secure_noauto_with_data(const ScanResultRow& _scan_result_row);
bool is_from_same_nameserver_ip(const ScanResultRow& _scan_result_row, const DomainState& _domain_state);

} // namespace Fred::Akm
} // namespace Fred

#endif
