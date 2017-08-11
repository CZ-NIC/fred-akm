#ifndef SCAN_TASK_HH_4DFE7A9913A3127ED90AA40A1038E9F9//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define SCAN_TASK_HH_4DFE7A9913A3127ED90AA40A1038E9F9

#include "src/domain.hh"

#include <string>
#include <set>
#include <unordered_map>

#define BOOST_RESULT_OF_USE_DECLTYPE
#include <boost/iterator/transform_iterator.hpp>


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
    typename It::value_type::second_type operator()(typename It::value_type& _value) const
    {
        return _value.second;
    }
    typename It::value_type::second_type operator()(const typename It::value_type& _value) const
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
};


} // namespace Akm
} // namespace Fred

#endif//SCAN_TASK_HH_4DFE7A9913A3127ED90AA40A1038E9F9
