#ifndef SCAN_TASK_HH_4DFE7A9913A3127ED90AA40A1038E9F9//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define SCAN_TASK_HH_4DFE7A9913A3127ED90AA40A1038E9F9

#include "src/domain.hh"

#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>

#define BOOST_RESULT_OF_USE_DECLTYPE
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator_range.hpp>


namespace Fred {
namespace Akm {


typedef std::string Nameserver;

struct ScanTask
{
    ScanTask(const Domain& _domain, const Nameserver& _nameserver)
        : domain(_domain), nameserver(_nameserver)
    {
    }

    Domain domain;
    Nameserver nameserver;
};


typedef std::set<Nameserver> Nameservers;

struct DomainScanTask
{
    DomainScanTask()
    {
    }

    DomainScanTask(const Domain& _domain, const Nameservers& _nameservers)
        : domain(_domain), nameservers(_nameservers)
    {
    }

    Domain domain;
    Nameservers nameservers;
};


namespace Detail {


template<class It>
struct get_value
{
    const typename It::value_type::second_type& operator()(const typename It::value_type& _value) const
    {
        return _value.second;
    }
};

template<class It>
auto make_map_value_iterator(It _it) -> decltype(boost::make_transform_iterator(_it, get_value<It>()))
{
    return boost::make_transform_iterator(_it, get_value<It>());
}


}


class DomainScanTaskCollection
{
private:
    std::unordered_map<std::string, DomainScanTask> task_by_domain_name_;
    std::unordered_set<Nameserver> nameservers_;

public:
    DomainScanTaskCollection();

    void insert_or_replace(const DomainScanTask& _task);

    void insert_or_update(const DomainScanTask& _task);

    void insert_or_update(const Domain& _domain, const Nameserver& _nameserver);

    void merge(const DomainScanTaskCollection& _other);

    void swap(DomainScanTaskCollection& _other);

    auto begin() const -> decltype(Detail::make_map_value_iterator(task_by_domain_name_.begin()));

    auto end() const -> decltype(Detail::make_map_value_iterator(task_by_domain_name_.end()));

    auto size() const -> decltype(task_by_domain_name_.size());

    std::size_t domain_size() const;

    std::size_t nameserver_size() const;

    void clear();

    const DomainScanTask& find(const std::string& _domain_name) const;
};


class NameserverToDomainScanTaskAdapter
{
private:
    std::unordered_map<Nameserver, std::unordered_set<const DomainScanTask*>> tasks_by_nameserver_;

public:
    NameserverToDomainScanTaskAdapter(const DomainScanTaskCollection& _scan_tasks)
    {
        for (const auto& task : _scan_tasks)
        {
            for (const auto& nameserver : task.nameservers)
            {
                tasks_by_nameserver_[nameserver].insert(&task);
            }
        }
    }

    NameserverToDomainScanTaskAdapter(const DomainScanTaskCollection&&) = delete;


    auto find_all(const Nameserver& _nameserver) const -> decltype(
        boost::make_iterator_range(
            tasks_by_nameserver_.at(_nameserver).begin(),
            tasks_by_nameserver_.at(_nameserver).end()
        )
    )
    {
        return boost::make_iterator_range(
            tasks_by_nameserver_.at(_nameserver).begin(),
            tasks_by_nameserver_.at(_nameserver).end()
        );
    }

    auto begin() const -> decltype(tasks_by_nameserver_.begin())
    {
        return tasks_by_nameserver_.begin();
    }

    auto end() const -> decltype(tasks_by_nameserver_.end())
    {
        return tasks_by_nameserver_.end();
    }
};


} // namespace Akm
} // namespace Fred

#endif//SCAN_TASK_HH_4DFE7A9913A3127ED90AA40A1038E9F9
