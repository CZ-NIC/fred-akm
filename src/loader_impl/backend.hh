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
#ifndef BACKEND_HH_DF70BFC9D12CF33FF8BAAB1131ABA697//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define BACKEND_HH_DF70BFC9D12CF33FF8BAAB1131ABA697

#include "src/i_loader.hh"
#include "src/i_akm.hh"

namespace Fred {
namespace Akm {


class BackendLoader : public ILoader
{
public:
    BackendLoader(const IAkm& _akm_backend);

    void load_insecure_tasks(DomainScanTaskCollection& _scan_tasks) const;

    void load_secure_auto_tasks(DomainScanTaskCollection& _scan_tasks) const;

    void load_secure_noauto_tasks(DomainScanTaskCollection& _scan_tasks) const;

private:
    const IAkm& akm_backend_;
};


} //namespace Akm
} //namespace Fred

#endif//BACKEND_HH_DF70BFC9D12CF33FF8BAAB1131ABA697
