#include "src/scan_task.hh"

namespace Fred {
namespace Akm {


DomainScanTaskCollection::DomainScanTaskCollection()
{
}


void DomainScanTaskCollection::insert_or_replace(const DomainScanTask& _task)
{
    task_by_domain_name_[_task.domain.fqdn] = _task;
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


} // namespace Akm
} // namespace Fred
