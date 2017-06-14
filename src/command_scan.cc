#include "src/command_scan.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner)
{
    auto tasks = _storage.get_scan_queue_tasks();

    _scanner.add_tasks(tasks);
    _scanner.scan([](const ScanResult& a){ });
}


} // namespace Akm
} // namespace Fred
