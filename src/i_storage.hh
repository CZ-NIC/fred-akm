#ifndef I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB

#include "src/i_scanner.hh"
#include "src/nameserver_domains.hh"
#include "src/scan_result_row.hh"
#include "src/notified_domain_status.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>

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

    virtual void save_scan_results(const std::vector<ScanResult>& _results, const NameserverDomainsCollection& _tasks, long long _iteration_id) const = 0;

    virtual long long start_new_scan_iteration() const = 0;

    virtual void end_scan_iteration(const long long _iteration_id) const = 0;

    virtual void wipe_unfinished_scan_iterations() const = 0;

    virtual long long prune_finished_scan_queue() const = 0;

    virtual ScanResultRows get_insecure_scan_result_rows_for_notify(
            int _seconds_back,
            bool _notify_from_last_iteration_only,
            bool _align_to_start_of_day = false) const = 0;

    virtual ScanResultRows get_insecure_scan_result_rows_for_update(
            int _seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual ScanResultRows get_secure_scan_result_rows_for_update(
            int _seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual void set_notified_domain_status(const NotifiedDomainStatus& _notified_domain_status) const = 0;

    virtual boost::optional<NotifiedDomainStatus> get_last_notified_domain_status(
            unsigned long long _domain_id) const = 0;

    virtual void clean_scan_results(
            const int _keep_seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual int get_current_unix_time() const = 0;
};


} // namespace Fred::Akm
} // namespace Fred

#endif//I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB
