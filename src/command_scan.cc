#include "src/command_scan.hh"

namespace Fred {
namespace Akm {


void command_scan(const IStorage& _storage, IScanner& _scanner)
{
    std::vector<NameserverDomains> tasks = {
        NameserverDomains(
            "ns1.fooo.cz",
            {Domain(1, "domain1.cz"), Domain(2, "domain2.cz"), Domain(3, "domain3.cz")}
        ),
        NameserverDomains(
            "ns2.baz.cz",
            {Domain(1, "domain1.cz"),}
        ),

    };

    _scanner.add_tasks(tasks);
    _scanner.scan([](const ScanResult& a){ });
}


} // namespace Akm
} // namespace Fred
