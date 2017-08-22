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

#include "src/scan_result_row.hh"

#include <istream>
#include <string>
#include <vector>

namespace Fred {
namespace Akm {


namespace {

std::string quote(const std::string& str)
{
    return "\"" + str + "\"";
}

std::string quote(int value)
{
    return std::to_string(value);
}

} // namespace Fred::Akim::{anonymous}

// see "src/sqlite/storage.cc"

std::string to_string(const ScanResultRow& scan_result_row)
{
    static const std::string delim = ", ";
    return  "[" +
            quote(scan_result_row.id) + delim +
            quote(scan_result_row.scan_iteration_id) + delim +
            quote(to_string(scan_result_row.scan_at)) + delim +
            quote(to_string(scan_result_row.scan_type)) + delim +
            quote(scan_result_row.cdnskey.status) + delim +
            quote(scan_result_row.domain_id) + delim +
            quote(scan_result_row.domain_name) + delim +
            quote(scan_result_row.nameserver) + delim +
            quote(scan_result_row.nameserver_ip) + delim +
            quote(scan_result_row.cdnskey.flags) + delim +
            quote(scan_result_row.cdnskey.proto) + delim +
            quote(scan_result_row.cdnskey.alg) + delim +
            quote(scan_result_row.cdnskey.public_key) +
            "]";
}

bool is_valid(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.id < 0) {
        return false;
    }
    if (_scan_result_row.scan_iteration_id < 0) {
        return false;
    }
    if (_scan_result_row.scan_at.empty()) {
        return false;
    }
    if (_scan_result_row.domain_id < 0) {
        return false;
    }
    if (_scan_result_row.domain_name.empty()) {
        return false;
    }
    if (_scan_result_row.cdnskey.status == "insecure") {
        if (_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_valid(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "insecure-empty") {
        if (_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_empty(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "secure") {
        if (!_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (!_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_valid(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "secure-empty") {
        if (!_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (!_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_empty(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "unresolved") {
        if (_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_empty(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "unresolved-ip") {
        if (_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (!_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_empty(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "untrustworthy") {
        if (!_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (!_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_empty(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else if (_scan_result_row.cdnskey.status == "unknown") {
        if (!_scan_result_row.nameserver.empty()) {
            return false;
        }
        if (!_scan_result_row.nameserver_ip.empty()) {
            return false;
        }
        if (!is_empty(_scan_result_row.cdnskey)) {
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}

bool is_insecure(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.scan_type == ScanType::insecure &&
        (_scan_result_row.cdnskey.status == "insecure" ||
         _scan_result_row.cdnskey.status == "insecure-empty" ||
         _scan_result_row.cdnskey.status == "unresolved" ||
         _scan_result_row.cdnskey.status == "unresolved-ip"))
    {
        return true;
    }
    return false;
}

bool is_insecure_with_data(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.scan_type == ScanType::insecure &&
        (_scan_result_row.cdnskey.status == "insecure" ||
         _scan_result_row.cdnskey.status == "insecure-empty"))
    {
        return true;
    }
    return false;
}

bool is_secure_auto(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.scan_type == ScanType::secure_auto &&
        (_scan_result_row.cdnskey.status == "secure" ||
         _scan_result_row.cdnskey.status == "secure-empty" ||
         _scan_result_row.cdnskey.status == "untrustworthy" ||
         _scan_result_row.cdnskey.status == "unknown"))
    {
        return true;
    }
    return false;
}

bool is_secure_noauto(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.scan_type == ScanType::secure_noauto &&
        (_scan_result_row.cdnskey.status == "secure" ||
         _scan_result_row.cdnskey.status == "secure-empty" ||
         _scan_result_row.cdnskey.status == "untrustworthy" ||
         _scan_result_row.cdnskey.status == "unknown"))
    {
        return true;
    }
    return false;
}

bool is_secure_auto_with_data(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.scan_type == ScanType::secure_auto &&
        (_scan_result_row.cdnskey.status == "secure" ||
         _scan_result_row.cdnskey.status == "secure-empty"))
    {
        return true;
    }
    return false;
}

bool is_secure_noauto_with_data(const ScanResultRow& _scan_result_row)
{
    if (_scan_result_row.scan_type == ScanType::secure_noauto &&
        (_scan_result_row.cdnskey.status == "secure" ||
         _scan_result_row.cdnskey.status == "secure-empty"))
    {
        return true;
    }
    return false;
}

bool is_from_same_nameserver_ip(const ScanResultRow& _scan_result_row, const DomainState& _domain_state)
{
    if (_scan_result_row.domain_id != _domain_state.domain.id)
    {
        return false;
    }
    if (_scan_result_row.domain_name != _domain_state.domain.fqdn)
    {
        return false;
    }
    if (_scan_result_row.scan_type != _domain_state.domain.scan_type)
    {
        return false;
    }
    if (_scan_result_row.nameserver != _domain_state.nameserver)
    {
        return false;
    }
    if (_scan_result_row.nameserver_ip != _domain_state.nameserver_ip)
    {
        return false;
    }
    return true;
}

} // namespace Fred::Akm
} // namespace Fred
