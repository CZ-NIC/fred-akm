#include <memory>

#include "src/utils.hh"
#include "src/log.hh"
#include "src/command_load.hh"

namespace Fred {
namespace Akm {


void command_load(
    const IStorage& _storage,
    const ILoader& _loader,
    std::unique_ptr<ILoaderOutputFilter> _filter,
    int _flags)
{
    NameserverDomainsCollection data;
    _loader.load_domains(data);

    if (_filter)
    {
        _filter->apply(data);
        _flags |= LoadFlags::WIPE_QUEUE;
        log()->info("forcing scan queue wipe on filtered input");
    }

    if (_flags & LoadFlags::WIPE_QUEUE)
    {
        _storage.wipe_scan_queue();
    }
    if (_flags & LoadFlags::ALLOW_DUPS)
    {
        _storage.append_to_scan_queue(data);
    }
    else
    {
        _storage.append_to_scan_queue_if_not_exists(data);
    }
    if (_flags & LoadFlags::PRUNE)
    {
        _storage.prune_scan_queue();
    }

    log()->info("imported to database");
}


} //namespace Akm
} //namespace Fred
