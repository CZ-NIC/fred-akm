#include "src/log.hh"
#include "src/command_scan.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner)
{
    _storage.wipe_unfinished_scan_iterations();

    auto tasks = _storage.get_scan_queue_tasks();
    log()->info("loaded scan queue ({} namserver(s))", tasks.size());

    long iteration_id = _storage.start_new_scan_iteration();
    log()->info("started new scan iteration (id={})", iteration_id);
    _scanner.scan(tasks, [&_storage, &iteration_id](const std::vector<ScanResult>& _results)
        { _storage.save_scan_results(_results, iteration_id); }
    );
    _storage.end_scan_iteration(iteration_id);
    log()->info("scan iteration finished (id={})", iteration_id);
}


} // namespace Akm
} // namespace Fred
