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
#ifndef I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB

#include "src/i_scanner.hh"
#include "src/scan_task.hh"
#include "src/scan_result_row.hh"
#include "src/domain_notified_status.hh"
#include "src/scan_type.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Akm {


class IStorage
{
public:
    virtual void append_to_scan_queue(const DomainScanTaskCollection& _data) const = 0;

    virtual void append_to_scan_queue_if_not_exists(const DomainScanTaskCollection& _data) const = 0;

    virtual void wipe_scan_queue() const = 0;

    virtual void prune_scan_queue() const = 0;

    virtual DomainScanTaskCollection get_scan_queue_tasks() const = 0;

    virtual void save_scan_results(const ScanResults& _results, const DomainScanTaskCollection& _tasks, long long _iteration_id) const = 0;

    virtual long long start_new_scan_iteration() const = 0;

    virtual void end_scan_iteration(const long long _iteration_id) const = 0;

    virtual void wipe_unfinished_scan_iterations() const = 0;

    virtual long long prune_finished_scan_queue() const = 0;

    virtual ScanResultRows get_scan_result_rows_of_akm_insecure_candidates_for_akm_notify(
            int _seconds_back,
            bool _notify_from_last_iteration_only,
            bool _align_to_start_of_day = false) const = 0;

    virtual ScanResultRows get_scan_result_rows_of_akm_insecure_candidates_for_akm_turn_on(
            int _seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual ScanResultRows get_scan_result_rows_of_akm_secure_candidates_for_akm_turn_on(
            int _seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual ScanResultRows get_scan_result_rows_of_akm_members_for_update(
            int _seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual void set_domain_notified_status(const DomainNotifiedStatus& _domain_notified_status) const = 0;

    virtual boost::optional<DomainNotifiedStatus> get_domain_last_notified_status(
            unsigned long long _domain_id) const = 0;

    virtual void clean_scan_results(
            const int _keep_seconds_back,
            bool _align_to_start_of_day = false) const = 0;

    virtual unsigned int get_current_unix_time() const = 0;
};


} // namespace Fred::Akm
} // namespace Fred

#endif//I_STORAGE_HH_1B55415E3EB327260D5DA07399BAA5FB
