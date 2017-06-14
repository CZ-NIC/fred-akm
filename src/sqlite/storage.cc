#include <iostream>
#include "src/sqlite/storage.hh"

namespace Fred {
namespace Akm {
namespace Sqlite {

namespace {


void create_schema_scan_queue(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_queue ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " import_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " nameserver TEXT NOT NULL,"
            " domain_id INTEGER NOT NULL,"
            " domain_name TEXT NOT NULL)"
    );

    _db.execute("CREATE INDEX scan_queue_ns_domain_idx ON scan_queue(nameserver, domain_id, domain_name)");
}


void create_schema_scan_result(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_task ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " scan_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " nameserver TEXT NOT NULL,"
            " domain_id INTEGER NOT NULL,"
            " domain_name TEXT NOT NULL)"
    );

    _db.execute("CREATE INDEX scan_task_scan_at_idx ON scan_task(scan_at)");
    _db.execute("CREATE INDEX scan_task_ns_domain_idx ON scan_task(nameserver, domain_id, domain_name)");

    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_result ("
            " scan_task_id INTEGER NOT NULL,"
            " nameserver_ip TEXT NOT NULL,"
            " cdnskey_status TEXT NOT NULL,"
            " cdnskey_alg INTEGER,"
            " cdnskey_public_key TEXT,"
            " FOREIGN KEY (scan_task_id) REFERENCES scan_task(id),"
            " UNIQUE (scan_task_id, nameserver_ip, cdnskey_status, cdnskey_alg, cdnskey_public_key))"
    );
}


void create_schema_scan_notification(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_notification ("
            " domain_id INTEGER PRIMARY KEY NOT NULL,"
            " last_at TEXT,"
            " FOREIGN KEY (domain_id) REFERENCES scan_task(domain_id))"
    );
}


void create_schema(sqlite3pp::database& _db)
{
    create_schema_scan_queue(_db);
    create_schema_scan_result(_db);
    create_schema_scan_notification(_db);
}


void drop_schema(sqlite3pp::database& _db)
{
    _db.execute("DROP TABLE IF EXISTS scan_queue");
    _db.execute("DROP TABLE IF EXISTS scan_result");
    _db.execute("DROP TABLE IF EXISTS scan_task");
    _db.execute("DROP TABLE IF EXISTS scan_notification");
}


}


namespace Impl {


void append_to_scan_queue(sqlite3pp::database& _db, const std::vector<NameserverDomains>& _data)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO scan_queue (nameserver, domain_id, domain_name)"
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
}


void append_to_scan_queue_if_not_exists(sqlite3pp::database& _db, const std::vector<NameserverDomains>& _data)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO scan_queue (nameserver, domain_id, domain_name)"
        " VALUES (?, ?, ?)"
    );
    sqlite3pp::query check(_db);
    check.prepare(
        "SELECT count(*) FROM scan_queue WHERE nameserver = ? AND domain_id = ? AND domain_name = ?"
    );
    for (const auto& ns_domains : _data)
    {
        for (const auto& domain : ns_domains.nameserver_domains)
        {
            check.bind(1, ns_domains.nameserver, sqlite3pp::nocopy);
            check.bind(2, static_cast<long long>(domain.id));
            check.bind(3, domain.fqdn, sqlite3pp::nocopy);
            auto count = (*check.begin()).get<long long>(0);
            if (count == 0)
            {
                insert.binder() << ns_domains.nameserver << static_cast<long long>(domain.id) << domain.fqdn;
                insert.step();
                insert.reset();
            }
            check.reset();
        }
    }
}

void clear_scan_queue(sqlite3pp::database& _db)
{
    _db.execute("DROP TABLE IF EXISTS scan_queue");
    create_schema_scan_queue(_db);
}


}


SqliteStorage::SqliteStorage(const std::string& _filename)
    : filename_(_filename)
{
}



void SqliteStorage::append_to_scan_queue(const std::vector<NameserverDomains>& _data) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::append_to_scan_queue(db, _data);
    xct.commit();
}


void SqliteStorage::append_to_scan_queue_if_not_exists(const std::vector<NameserverDomains>& _data) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::append_to_scan_queue_if_not_exists(db, _data);
    xct.commit();
}


void SqliteStorage::wipe_scan_queue() const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    db.execute("DROP TABLE IF EXISTS scan_queue");
    create_schema_scan_queue(db);
    xct.commit();
}


void SqliteStorage::prune_scan_queue() const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    db.execute(
        "DELETE FROM scan_queue WHERE"
        " id IN"
        " (SELECT DISTINCT older.id FROM scan_queue older"
        " JOIN scan_queue newer ON newer.domain_name = older.domain_name"
        " AND newer.import_at > older.import_at AND newer.domain_id != older.domain_id)"
    );
    xct.commit();
}


std::vector<NameserverDomains> SqliteStorage::get_scan_queue_tasks() const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    sqlite3pp::query tasks_query(
        db,
        "SELECT nameserver, domain_id, domain_name"
        " FROM scan_queue ORDER BY nameserver, domain_name ASC"
    );

    std::vector<NameserverDomains> tasks;
    NameserverDomains ns_domains;
    for (auto row : tasks_query)
    {
        std::string nameserver;
        long long domain_id;
        std::string domain_name;
        row.getter() >> nameserver >> domain_id >> domain_name;

        if (ns_domains.nameserver != nameserver)
        {
            if (!ns_domains.nameserver.empty())
            {
                tasks.push_back(ns_domains);
                ns_domains.nameserver_domains.clear();
            }
            ns_domains.nameserver = nameserver;
        }
        ns_domains.nameserver_domains.emplace_back(Domain(domain_id, domain_name));
    }

    return tasks;
}


} // namespace Sqlite
} // namespace Akm
} // namespace Fred
