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
#include "src/scan_task.hh"

namespace Fred {
namespace Akm {


DomainScanTaskCollection::DomainScanTaskCollection()
{
}


void DomainScanTaskCollection::insert_or_replace(const DomainScanTask& _task)
{
    task_by_domain_name_[_task.domain.fqdn] = _task;
    nameservers_.insert(_task.nameservers.begin(), _task.nameservers.end());
}


void DomainScanTaskCollection::insert_or_update(const DomainScanTask& _task)
{
    auto search_it = task_by_domain_name_.find(_task.domain.fqdn);
    if (search_it == task_by_domain_name_.end())
    {
        insert_or_replace(_task);
    }
    else
    {
        DomainScanTask& task_found = search_it->second;
        if (task_found.domain == _task.domain)
        {
            task_found.nameservers.insert(_task.nameservers.begin(), _task.nameservers.end());
            nameservers_.insert(_task.nameservers.begin(), _task.nameservers.end());
        }
        else
        {
            insert_or_replace(_task);
        }
    }
}


void DomainScanTaskCollection::insert_or_update(const Domain& _domain, const Nameserver& _nameserver)
{
    insert_or_update(DomainScanTask(_domain, {_nameserver}));
}


void DomainScanTaskCollection::merge(const DomainScanTaskCollection& _other)
{
    for (const auto& task : _other)
    {
        insert_or_update(task);
    }
}


void DomainScanTaskCollection::swap(DomainScanTaskCollection& _other)
{
    task_by_domain_name_.swap(_other.task_by_domain_name_);
}


auto DomainScanTaskCollection::begin() const -> decltype(Detail::make_map_value_iterator(task_by_domain_name_.begin()))
{
    return Detail::make_map_value_iterator(task_by_domain_name_.begin());
}


auto DomainScanTaskCollection::end() const -> decltype(Detail::make_map_value_iterator(task_by_domain_name_.end()))
{
    return Detail::make_map_value_iterator(task_by_domain_name_.end());
}


auto DomainScanTaskCollection::size() const -> decltype(task_by_domain_name_.size())
{
    return task_by_domain_name_.size();
}


std::size_t DomainScanTaskCollection::domain_size() const
{
    return size();
}


std::size_t DomainScanTaskCollection::nameserver_size() const
{
    return nameservers_.size();
}


void DomainScanTaskCollection::clear()
{
    task_by_domain_name_.clear();
    nameservers_.clear();
}


const DomainScanTask& DomainScanTaskCollection::find(const std::string& _domain_name) const
{
    return task_by_domain_name_.at(_domain_name);
}


} // namespace Akm
} // namespace Fred
