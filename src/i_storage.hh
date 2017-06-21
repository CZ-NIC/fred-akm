#ifndef I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB

#include <string>
#include <vector>

#include "src/nameserver_domains.hh"
#include "src/i_scanner.hh"

namespace Fred {
namespace Akm {


class IStorage
{
public:
    virtual void append_to_scan_queue(const NameserverDomainsCollection& _data) const = 0;

    virtual void append_to_scan_queue_if_not_exists(const NameserverDomainsCollection& _data) const = 0;

    virtual void wipe_scan_queue() const = 0;

    virtual void prune_scan_queue() const = 0;

    virtual NameserverDomainsCollection get_scan_queue_tasks() const = 0;

    virtual void save_scan_results(const std::vector<ScanResult>& _results, long long _iteration_id) const = 0;

    virtual long long start_new_scan_iteration() const = 0;

    virtual void end_scan_iteration(const long long _iteration_id) const = 0;

    virtual void wipe_unfinished_scan_iterations() const = 0;

    virtual long long prune_finished_scan_queue() const = 0;
};


} // namespace Fred
} // namespace Akm

#endif//I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB
