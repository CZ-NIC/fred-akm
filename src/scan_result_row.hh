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

#include <istream>
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
          scan_at_seconds(),
          domain_id(),
          domain_name(),
          has_keyset(),
          nameserver(),
          nameserver_ip(),
          cdnskey()
    {
    }

    ScanResultRow(
            int _id,
            int _scan_iteration_id,
            const std::string& _scan_at,
            int _scan_at_seconds,
            int _domain_id,
            const std::string& _domain_name,
            bool _has_keyset,
            const std::string& _nameserver,
            const std::string& _nameserver_ip,
            const Cdnskey& _cdnskey)
        : id(_id),
          scan_iteration_id(_scan_iteration_id),
          scan_at(_scan_at),
          scan_at_seconds(_scan_at_seconds),
          domain_id(_domain_id),
          domain_name(_domain_name),
          has_keyset(_has_keyset),
          nameserver(_nameserver),
          nameserver_ip(_nameserver_ip),
          cdnskey(_cdnskey)
    {
    }

    ScanResultRow(const ScanResultRow& _scan_result_row)
        : id(_scan_result_row.id),
          scan_iteration_id(_scan_result_row.scan_iteration_id),
          scan_at(_scan_result_row.scan_at),
          scan_at_seconds(_scan_result_row.scan_at_seconds),
          domain_id(_scan_result_row.domain_id),
          domain_name(_scan_result_row.domain_name),
          has_keyset(_scan_result_row.has_keyset),
          nameserver(_scan_result_row.nameserver),
          nameserver_ip(_scan_result_row.nameserver_ip),
          cdnskey(_scan_result_row.cdnskey)
    {
    }

    int id;
    int scan_iteration_id;
    std::string scan_at;
    int scan_at_seconds;
    int domain_id;
    std::string domain_name;
    bool has_keyset;
    std::string nameserver;
    std::string nameserver_ip;
    Cdnskey cdnskey;
};

typedef std::vector<ScanResultRow> ScanResultRows;

std::ostream& operator<<(std::ostream& os, const ScanResultRow& scan_result_row);
std::istream& operator>>(std::istream& is, ScanResultRow& scan_result_row);
std::string to_string(const ScanResultRow& scan_result_row);

bool is_valid(const ScanResultRow& _scan_result_row);
bool is_insecure(const ScanResultRow& _scan_result_row);
bool is_insecure_with_data(const ScanResultRow& _scan_result_row);
bool is_secure(const ScanResultRow& _scan_result_row);
bool is_from_same_nameserver_ip(const ScanResultRow& _scan_result_row, const DomainState& _domain_state);

} // namespace Fred::Akm
} // namespace Fred

#endif
