/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include <algorithm>
#include <functional>

#include "src/scan_task.hh"
#include "src/log.hh"
#include "src/command_scan.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner, bool batch_mode)
{
    _storage.wipe_unfinished_scan_iterations();
    auto removed_tasks = _storage.prune_finished_scan_queue();
    log()->info("removed {} finished tasks from scan queue", removed_tasks);

    auto scan_tasks = _storage.get_scan_queue_tasks();
    log()->info("loaded scan queue ({} namserver(s))", scan_tasks.size());
    if (scan_tasks.size() == 0)
    {
        log()->info("queue empty...");
        return;
    }

    auto run_batch_scan = [&_storage, &_scanner](const DomainScanTaskCollection& _scan_batch)
    {
        const auto iteration_id = _storage.start_new_scan_iteration();
        log()->info("started new scan iteration (id={})", iteration_id);
        _scanner.scan(_scan_batch, [&_storage, &_scan_batch, &iteration_id](const std::vector<ScanResult>& _results)
            { _storage.save_scan_results(_results, _scan_batch, iteration_id); }
        );
        _storage.end_scan_iteration(iteration_id);
        log()->info("scan iteration finished (id={})", iteration_id);
    };

    if (!batch_mode)
    {
        run_batch_scan(scan_tasks);
    }
    else
    {
        const auto BATCH_NS_MIN = 2000UL;
        const auto BATCH_DS_MIN = 10000UL;
        auto batch_ns_max = std::max(scan_tasks.nameserver_size() / 10, BATCH_NS_MIN);
        auto batch_ds_max = std::max(scan_tasks.domain_size() / 50, BATCH_DS_MIN);
        log()->debug("ns total:{} batch-max:{}", scan_tasks.nameserver_size(), batch_ns_max);
        log()->debug("ds total:{} batch-max:{}", scan_tasks.domain_size(), batch_ds_max);

        DomainScanTaskCollection scan_batch;
        for (const auto& task : scan_tasks)
        {
            scan_batch.insert_or_update(task);
            if (scan_batch.nameserver_size() >= batch_ns_max || scan_batch.domain_size() >= batch_ds_max)
            {
                run_batch_scan(scan_batch);
                scan_batch.clear();
            }
        }
        run_batch_scan(scan_batch);
    }

    log()->debug("all done");
}


} // namespace Akm
} // namespace Fred
