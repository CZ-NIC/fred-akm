#include "src/sqlite/storage.hh"

namespace Fred {
namespace Akm {
namespace Sqlite {

void SqliteStorage::create_schema(sqlite3pp::database& _db) const
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_task ("
            "id integer primary key autoincrement not null, "
            "nameserver text not null, "
            "domain_id integer not null, "
            "domain_name text not null)"
    );
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_result ("
            "id integer not null, "
            "ns_ip_address text not null, "
            "cdnskey_status text not null, "
            "cdnskey_algorithm text, "
            "cdnskey_public_key text, "
            "FOREIGN KEY(id) REFERENCES scan_task(id))"
    );
}


void SqliteStorage::drop_schema(sqlite3pp::database& _db) const
{
    _db.execute("DROP TABLE IF EXISTS scan_result");
    _db.execute("DROP TABLE IF EXISTS scan_task");
}


SqliteStorage::SqliteStorage(const std::string& _filename)
    : filename_(_filename)
{
}


void SqliteStorage::new_scan_tasks(const std::vector<NameserverDomains>& _data) const
{
    sqlite3pp::database db(filename_.c_str());
    drop_schema(db);
    create_schema(db);

    sqlite3pp::transaction xct(db);
    sqlite3pp::command insert(db);
    insert.prepare(
        "INSERT INTO scan_task (nameserver, domain_id, domain_name)"
        " VALUES (?, ?, ?)"
    );
    for (const auto& ns_domains : _data)
    {
        for (const auto& domain : ns_domains.nameserver_domains)
        {
            insert.binder() << ns_domains.nameserver << static_cast<long long>(domain.id) << domain.fqdn; 
            insert.step();
            insert.reset();
        }
    }
    xct.commit();
}

} // namespace Sqlite
} // namespace Akm
} // namespace Fred
