#include "src/command_scan.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner)
{
    auto tasks = _storage.get_scan_queue_tasks();

    _scanner.scan(tasks, [&_storage](const std::vector<ScanResult>& _results)
        { _storage.save_scan_results(_results); }
    );
}


} // namespace Akm
} // namespace Fred
