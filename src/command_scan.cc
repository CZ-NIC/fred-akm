#include "src/command_scan.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner)
{
    auto tasks = _storage.get_scan_queue_tasks();

    _scanner.add_tasks(tasks);
    _scanner.scan([&_storage](const ScanResult& _result){ _storage.save_scan_result(_result); });
}


} // namespace Akm
} // namespace Fred
