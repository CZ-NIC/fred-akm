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
#include "src/loader_impl/backend.hh"
#include "src/log.hh"

namespace Fred {
namespace Akm {


BackendLoader::BackendLoader(const IAkm& _akm_backend)
    : akm_backend_(_akm_backend)
{
}


void BackendLoader::load_insecure_tasks(DomainScanTaskCollection& _scan_tasks) const
{
    DomainScanTaskCollection tasks = akm_backend_.get_nameservers_with_insecure_automatically_managed_domain_candidates();
    _scan_tasks.merge(tasks);
    log()->info("loaded insecure tasks from backend ({} domain(s))", tasks.size());
}


void BackendLoader::load_secure_auto_tasks(DomainScanTaskCollection& _scan_tasks) const
{
    DomainScanTaskCollection tasks = akm_backend_.get_nameservers_with_automatically_managed_domains();
    _scan_tasks.merge(tasks);
    log()->info("loaded secure-auto tasks from backend ({} domain(s))", tasks.size());
}


void BackendLoader::load_secure_noauto_tasks(DomainScanTaskCollection& _scan_tasks) const
{
    DomainScanTaskCollection tasks = akm_backend_.get_nameservers_with_secure_automatically_managed_domain_candidates();
    _scan_tasks.merge(tasks);
    log()->info("loaded secure-noauto tasks from backend ({} domain(s))", tasks.size());
}


} //namespace Akm
} //namespace Fred
