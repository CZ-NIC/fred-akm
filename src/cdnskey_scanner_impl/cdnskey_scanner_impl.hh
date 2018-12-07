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
#ifndef EXTERNAL_CDNSKEY_SCANNER_IMPL_HH_26D42C56B1493D7951148C69A80A5BA9//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define EXTERNAL_CDNSKEY_SCANNER_IMPL_HH_26D42C56B1493D7951148C69A80A5BA9

#include "src/i_scanner.hh"

namespace Fred {
namespace Akm {


class ExternalCdnskeyScannerImpl : public IScanner
{
public:
    ExternalCdnskeyScannerImpl(const std::string& _scanner_path);

    void scan(const DomainScanTaskCollection& _tasks, OnResultsCallback _callback) const;

private:
    std::vector<std::string> scanner_path_;
};


} //namespace Akm
} //namespace Fred


#endif//EXTERNAL_CDNSKEY_SCANNER_IMPL_HH_26D42C56B1493D7951148C69A80A5BA9
