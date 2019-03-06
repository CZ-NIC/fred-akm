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
#ifndef I_SCANNER_H_BA9A1E563A15C26233F87B62E27EB0FD//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_SCANNER_H_BA9A1E563A15C26233F87B62E27EB0FD

#include <string>
#include <vector>
#include <boost/optional.hpp>

#include "src/scan_task.hh"

namespace Fred {
namespace Akm {


enum class CdnskeyStatus
{
    UNKNOWN,
    INSECURE,
    SECURE,
    UNTRUSTWORTHY,
    UNRESOLVED
};

struct ScanResult
{
    //CdnskeyStatus cdnskey_status;
    std::string cdnskey_status;
    std::string domain_name;
    boost::optional<std::string> nameserver;
    boost::optional<std::string> nameserver_ip;
    boost::optional<int> cdnskey_flags;
    boost::optional<int> cdnskey_proto;
    boost::optional<int> cdnskey_alg;
    boost::optional<std::string> cdnskey_public_key;
};


typedef std::vector<ScanResult> ScanResults;


class IScanner
{
public:
    typedef std::function<void(const std::vector<ScanResult>& _results)> OnResultsCallback;

    virtual void scan(const DomainScanTaskCollection& _tasks, OnResultsCallback _callback) const = 0;
};


} //namespace Akm
} //namespace Fred


#endif//I_SCANNER_H_BA9A1E563A15C26233F87B62E27EB0FD
