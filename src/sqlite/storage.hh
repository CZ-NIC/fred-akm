#ifndef STORAGE_HH_F547FDFC7F6B4D918928C606B9E4A87E//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define STORAGE_HH_F547FDFC7F6B4D918928C606B9E4A87E

#include "src/i_storage.hh"
#include "3rd_party/sqlite3pp/headeronly_src/sqlite3pp.h"

namespace Fred {
namespace Akm {
namespace Sqlite {


class SqliteStorage : public IStorage
{
public:
    SqliteStorage(const std::string& _filename);

    void append_to_scan_queue(const NameserverDomainsCollection& _data) const;

    void append_to_scan_queue_if_not_exists(const NameserverDomainsCollection& _data) const;

    void wipe_scan_queue() const;

    void prune_scan_queue() const;

    NameserverDomainsCollection get_scan_queue_tasks() const;

    void save_scan_results(const std::vector<ScanResult>& _results, long long _iteration_id) const;

    long long start_new_scan_iteration() const;

    void end_scan_iteration(const long long _iteration_id) const;

    void wipe_unfinished_scan_iterations() const;

private:
    sqlite3pp::database get_db() const;

    std::string filename_;
};


} // namespace Sqlite
} // namespace Akm
} // namespace Fred

#endif//STORAGE_HH_F547FDFC7F6B4D918928C606B9E4A87E
