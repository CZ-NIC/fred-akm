#include <algorithm>
#include <functional>

#include "src/nameserver_domains.hh"
#include "src/log.hh"
#include "src/command_scan.hh"

namespace Fred {
namespace Akm {

struct DomainHash
{
    size_t operator()(const Domain& _domain) const
    {
        return std::hash<std::string>()(_domain.fqdn)
            ^ std::hash<unsigned long long>()(_domain.id)
            ^ std::hash<bool>()(_domain.has_keyset);
    }
};


struct DomainEqual
{
    bool operator()(const Domain& _left, const Domain& _right) const
    {
        return _left.id == _right.id
            && _left.fqdn == _right.fqdn
            && _left.has_keyset == _right.has_keyset;
    }
};


void command_scan(const IStorage& _storage, IScanner& _scanner)
{
    _storage.wipe_unfinished_scan_iterations();

    auto tasks = _storage.get_scan_queue_tasks();
    log()->info("loaded scan queue ({} namserver(s))", tasks.size());
    if (tasks.size() == 0)
    {
        log()->info("queue empty...");
        return;
    }

    typedef std::vector<std::string> Nameservers;
    typedef std::unordered_map<Domain, Nameservers, DomainHash, DomainEqual> DomainNameserversMap;
    DomainNameserversMap inverse_tasks;
    for (const auto kv : tasks)
    {
        const auto& ns = kv.second.nameserver;
        const auto& ns_domains = kv.second.nameserver_domains;
        for (const auto& domain : ns_domains)
        {
            inverse_tasks[domain].push_back(ns);
        }
    }

    auto run_batch_scan = [&_storage, &_scanner](const NameserverDomainsCollection& _batch)
    {
        long iteration_id = _storage.start_new_scan_iteration();
        log()->info("started new scan iteration (id={})", iteration_id);
        _scanner.scan(_batch, [&_storage, &iteration_id](const std::vector<ScanResult>& _results)
            { _storage.save_scan_results(_results, iteration_id); }
        );
        _storage.end_scan_iteration(iteration_id);
        log()->info("scan iteration finished (id={})", iteration_id);
    };

    const auto BATCH_NS_MIN = 2000UL;
    const auto BATCH_DS_MIN = 10000UL;
    auto batch_ns_max = std::max(tasks.size() / 10, BATCH_NS_MIN);
    auto batch_ds_max = std::max(inverse_tasks.size() / 50, BATCH_DS_MIN);
    log()->debug("ns total:{} batch-max:{}", tasks.size(), batch_ns_max);
    log()->debug("ds total:{} batch-max:{}", inverse_tasks.size(), batch_ds_max);

    auto batch_domains_count = 0;

    NameserverDomainsCollection batch;
    auto it = inverse_tasks.begin();
    while (it != inverse_tasks.end())
    {
        const auto& domain = it->first;
        const auto& nameservers = it->second;
        for (const auto& ns : nameservers)
        {
            batch[ns].nameserver = ns;
            batch[ns].nameserver_domains.push_back(domain);
            batch_domains_count += 1;
        }
        if (batch.size() >= batch_ns_max || batch_domains_count >= batch_ds_max)
        {
            {
                run_batch_scan(batch);
            }

            batch_domains_count = 0;
            batch.clear();
        }
        ++it;
    }
    run_batch_scan(batch);
    log()->debug("all done");
}


} // namespace Akm
} // namespace Fred
