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

    void new_scan_tasks(const std::vector<NameserverDomains>& _data) const;

    void append_scan_tasks(const std::vector<NameserverDomains>& _data) const;

private:
    void create_schema(sqlite3pp::database& _db) const;

    void drop_schema(sqlite3pp::database& _db) const;

    std::string filename_;
};


} // namespace Sqlite
} // namespace Akm
} // namespace Fred

#endif//STORAGE_HH_F547FDFC7F6B4D918928C606B9E4A87E
