#include "src/sqlite/storage.hh"
#include "src/log.hh"

#include "src/notified_domain_state.hh"
#include "src/scan_result_row.hh"
#include "src/domain_state.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <stdexcept>

namespace Fred {
namespace Akm {
namespace Sqlite {

namespace {


void create_schema_scan_iteration(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_iteration ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " start_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " end_at TEXT DEFAULT NULL)"
    );
}


void create_schema_scan_queue(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_queue ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " import_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " nameserver TEXT NOT NULL,"
            " domain_id INTEGER NOT NULL CHECK(domain_id > 0),"
            " domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''),"
            " has_keyset BOOLEAN NOT NULL CHECK(has_keyset IN (0, 1)))"
    );

    _db.execute("CREATE INDEX scan_queue_ns_domain_idx ON scan_queue(nameserver, domain_id, domain_name)");
    _db.execute("CREATE INDEX scan_queue_domain_name_idx ON scan_queue(domain_name)");
}


void create_schema_scan_result(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_result ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " scan_iteration_id INTEGER NOT NULL,"
            " scan_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " domain_id INTEGER NOT NULL CHECK(domain_id > 0),"
            " domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''),"
            " has_keyset BOOLEAN NOT NULL CHECK(has_keyset IN (0, 1)),"
            " cdnskey_status TEXT NOT NULL CHECK(cdnskey_status IN ('insecure','insecure-empty','unresolved','secure','secure-empty','untrustworthy','unknown')),"
            " nameserver TEXT,"
            " nameserver_ip TEXT,"
            " cdnskey_flags INTEGER,"
            " cdnskey_proto INTEGER,"
            " cdnskey_alg INTEGER,"
            " cdnskey_public_key TEXT,"
            " FOREIGN KEY (scan_iteration_id) REFERENCES scan_iteration(id))"
    );
}


void create_schema_notified_domain_state(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS notified_domain_state ( "
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "domain_id INTEGER NOT NULL CHECK(domain_id > 0), "
            "domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''), "
            "has_keyset BOOLEAN NOT NULL CHECK(has_keyset IN (0, 1)), "
            "cdnskeys TEXT, "
            "notification_type INTEGER NOT NULL CHECK(notification_type IN (0, 1)), "
            "last_at TEXT NOT NULL DEFAULT (datetime('now')))"
    );
}


void create_schema(sqlite3pp::database& _db)
{

    create_schema_scan_iteration(_db);
    create_schema_scan_queue(_db);
    create_schema_scan_result(_db);
    create_schema_notified_domain_state(_db);
}


void drop_schema(sqlite3pp::database& _db)
{
    _db.execute("DROP TABLE IF EXISTS scan_queue");
    _db.execute("DROP TABLE IF EXISTS scan_result");
    _db.execute("DROP TABLE IF EXISTS scan");
    _db.execute("DROP TABLE IF EXISTS scan_iteration");
    _db.execute("DROP TABLE IF EXISTS notified_domain_state");
}


}


namespace Impl {


void append_to_scan_queue(sqlite3pp::database& _db, const NameserverDomainsCollection& _data)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO scan_queue (nameserver, domain_id, domain_name, has_keyset)"
        " VALUES (?, ?, ?, ?)"
    );
    for (const auto& kv : _data)
    {
        const auto& nameserver = kv.second.nameserver;
        for (const auto& domain : kv.second.nameserver_domains)
        {
            insert.binder() << nameserver << static_cast<long long>(domain.id) << domain.fqdn << domain.has_keyset;
            insert.step();
            insert.reset();
        }
    }
}


void append_to_scan_queue_if_not_exists(sqlite3pp::database& _db, const NameserverDomainsCollection& _data)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO scan_queue (nameserver, domain_id, domain_name, has_keyset)"
        " VALUES (?, ?, ?, ?)"
    );
    sqlite3pp::query check(_db);
    check.prepare(
        "SELECT 1 FROM scan_queue WHERE"
        " nameserver = :nameserver AND domain_id = :domain_id AND domain_name = :domain_name"
    );
    for (const auto& kv : _data)
    {
        const auto& nameserver = kv.second.nameserver;
        for (const auto& domain : kv.second.nameserver_domains)
        {
            check.bind(":nameserver", nameserver, sqlite3pp::nocopy);
            check.bind(":domain_id", static_cast<long long>(domain.id));
            check.bind(":domain_name", domain.fqdn, sqlite3pp::nocopy);
            if (check.begin() == check.end())
            {
                insert.binder() << nameserver << static_cast<long long>(domain.id) << domain.fqdn << domain.has_keyset;
                insert.step();
                insert.reset();
            }
            check.reset();
        }
    }
}

ScanResultRows get_insecure_scan_result_rows(sqlite3pp::database& _db, const int _seconds_back, const bool _notify_from_last_iteration_only)
{
    sqlite3pp::query query(_db);
    query.prepare(boost::str(boost::format(
        "SELECT id, "
               "scan_iteration_id, "
               "scan_at, "
               "strftime('%%s', datetime(scan_at)) as scan_at_seconds, "
               "domain_id, "
               "domain_name, "
               "has_keyset, "
               "cdnskey_status, "
               "nameserver, "
               "nameserver_ip, "
               "cdnskey_flags, "
               "cdnskey_proto, "
               "cdnskey_alg, "
               "cdnskey_public_key "
          "FROM scan_result "
         "WHERE scan_iteration_id IN "
             "(SELECT scan_iteration_id "
                "FROM scan_result "
               "WHERE scan_at BETWEEN datetime('now', '%1% seconds') AND datetime('now') "
               "GROUP BY scan_iteration_id) " // always get all scan_results from concrete iteration_id
           "AND has_keyset = 0 "
           "%2%"
         "ORDER BY scan_at_seconds DESC, id DESC")
                   % (_seconds_back * -1)
                   % (_notify_from_last_iteration_only ? "AND scan_iteration_id = (SELECT MAX(scan_iteration_id) FROM scan_result) " : "")).c_str());

    //query.bind(1, _seconds_back * -1); // TODO

    // Note: if (query.begin() == query.end()) increments internal pointer, do not use here!

    ScanResultRows scan_result;

    boost::optional<ScanResultRow> last_scan_result_row;
    for (auto row : query) {
        ScanResultRow scan_result_row;
        row.getter()
                >> scan_result_row.id
                >> scan_result_row.scan_iteration_id
                >> scan_result_row.scan_at
                >> scan_result_row.scan_at_seconds
                >> scan_result_row.domain_id
                >> scan_result_row.domain_name
                >> scan_result_row.has_keyset
                >> scan_result_row.cdnskey.status
                >> scan_result_row.nameserver
                >> scan_result_row.nameserver_ip
                >> scan_result_row.cdnskey.flags
                >> scan_result_row.cdnskey.proto
                >> scan_result_row.cdnskey.alg
                >> scan_result_row.cdnskey.public_key;
        scan_result.emplace_back(scan_result_row);
        std::cout << scan_result_row << std::endl;
    }
    if (scan_result.empty()) {
        throw std::runtime_error("no scan_results found");
    }
    return scan_result;
}

void set_notified_domain_state(sqlite3pp::database& _db, const NotifiedDomainState& _notified_domain_state)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO notified_domain_state (domain_id, domain_name, has_keyset, cdnskeys, notification_type, last_at) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    insert.binder()
            << static_cast<long long>(_notified_domain_state.domain_id)
            << _notified_domain_state.domain_name
            << _notified_domain_state.has_keyset
            << _notified_domain_state.serialized_cdnskeys
            << _notified_domain_state.notification_type
            << _notified_domain_state.last_at;
    insert.execute();
}

boost::optional<NotifiedDomainState> get_last_notified_domain_state(sqlite3pp::database& _db, const unsigned long long _domain_id)
{
    sqlite3pp::query query(_db);
    query.prepare(boost::str(boost::format(
        "SELECT domain_id, "
               "domain_name, "
               "has_keyset, "
               "cdnskeys, "
               "notification_type, "
               "last_at, "
               "strftime('%%s', datetime(last_at)) as last_at_seconds "
          "FROM notified_domain_state "
         "WHERE domain_id == '%1%' "
         "ORDER BY id DESC") % (static_cast<long long>(_domain_id))).c_str());

    //query.bind(1, _domain_id * -1); // TODO

    // how to check that the query is empty?
    // query.begin() == query.end() has some side-effect, if used, following (*query.begin()).getter() does not work
    // solution for now:
    for (auto row : query) {
        NotifiedDomainState notified_domain_state;
        long long domain_id;
        row.getter()
            >> domain_id
            >> notified_domain_state.domain_name
            >> notified_domain_state.has_keyset
            >> notified_domain_state.serialized_cdnskeys
            >> notified_domain_state.notification_type;
        notified_domain_state.domain_id = static_cast<unsigned long long>(domain_id);
        return notified_domain_state;
    }

    // if we got here, query was empty
    return boost::optional<NotifiedDomainState>();
}


void clear_scan_queue(sqlite3pp::database& _db)
{
    _db.execute("DROP TABLE IF EXISTS scan_queue");
    create_schema_scan_queue(_db);
}


}


sqlite3pp::database SqliteStorage::get_db() const
{
    sqlite3pp::database db(filename_.c_str());
    db.enable_foreign_keys(true);
    db.enable_triggers(true);
    return db;
}


SqliteStorage::SqliteStorage(const std::string& _filename)
    : filename_(_filename)
{
}


void SqliteStorage::append_to_scan_queue(const NameserverDomainsCollection& _data) const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::append_to_scan_queue(db, _data);
    xct.commit();
}


void SqliteStorage::append_to_scan_queue_if_not_exists(const NameserverDomainsCollection& _data) const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::append_to_scan_queue_if_not_exists(db, _data);
    xct.commit();
}

ScanResultRows SqliteStorage::get_insecure_scan_result_rows(const int _seconds_back, const bool _notify_from_last_iteration_only) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    return Impl::get_insecure_scan_result_rows(db, _seconds_back, _notify_from_last_iteration_only);
}

void SqliteStorage::set_notified_domain_state(const NotifiedDomainState& _notified_domain_state) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::set_notified_domain_state(db, _notified_domain_state);
    xct.commit();
}

boost::optional<NotifiedDomainState> SqliteStorage::get_last_notified_domain_state(const unsigned long long _domain_id) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    return Impl::get_last_notified_domain_state(db, _domain_id);
}

void SqliteStorage::wipe_scan_queue() const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    db.execute("DROP TABLE IF EXISTS scan_queue");
    create_schema_scan_queue(db);
    xct.commit();
}


void SqliteStorage::prune_scan_queue() const
{
    auto db = get_db();
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


NameserverDomainsCollection SqliteStorage::get_scan_queue_tasks() const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    sqlite3pp::query tasks_query(
        db,
        "SELECT nameserver, domain_id, domain_name, has_keyset"
        " FROM scan_queue ORDER BY nameserver, domain_name ASC"
    );

    NameserverDomainsCollection tasks;
    for (auto row : tasks_query)
    {
        std::string nameserver;
        long long domain_id;
        std::string domain_name;
        bool has_keyset;
        row.getter() >> nameserver >> domain_id >> domain_name >> has_keyset;

        Domain domain(domain_id, domain_name, has_keyset);
        auto& record = tasks[nameserver];
        record.nameserver = nameserver;
        record.nameserver_domains.emplace_back(domain);
    }

    return tasks;
}


void SqliteStorage::save_scan_results(const std::vector<ScanResult>& _results, long long _iteration_id) const
{
    if (_iteration_id <= 0)
    {
        throw std::runtime_error("invalid scan iteration id");
    }
    auto db = get_db();
    sqlite3pp::transaction xct(db);

    sqlite3pp::command i_result(db);
    i_result.prepare(
        "INSERT INTO scan_result"
        " (scan_iteration_id, domain_id, domain_name, has_keyset, cdnskey_status,"
        " nameserver, nameserver_ip, cdnskey_flags, cdnskey_proto, cdnskey_alg, cdnskey_public_key)"
        " VALUES (:scan_iteration_id, :domain_id, :domain_name, :has_keyset, :cdnskey_status, :nameserver, :nameserver_ip,"
        " :cdnskey_flags, :cdnskey_proto, :cdnskey_alg, :cdnskey_public_key)"
    );

    for (const auto& result : _results)
    {
        long long task_domain_id = 0;
        bool task_has_keyset = false;

        sqlite3pp::query s_queue(db,
            "SELECT id, domain_id, has_keyset FROM scan_queue WHERE domain_name = :domain_name"
            " AND nameserver = coalesce(:nameserver, nameserver) ORDER BY domain_id DESC"
        );
        s_queue.bind(":domain_name", result.domain_name, sqlite3pp::nocopy);
        if (result.nameserver)
        {
            s_queue.bind(":nameserver", *result.nameserver, sqlite3pp::nocopy);
        }
        else
        {
            s_queue.bind(":nameserver", sqlite3pp::null_type());
        }

        for (auto row : s_queue)
        {
            long long scan_queue_id = 0;
            long long domain_id = 0;
            row.getter() >> scan_queue_id >> domain_id >> task_has_keyset;
            if (task_domain_id == 0)
            {
                task_domain_id = domain_id;
            }
            /* extra check if there are different domain_id in queue (need to handle?) */
            else if (task_domain_id != domain_id)
            {
                log()->error(
                    "different domain id found for same domain name in scan queue ({} != {}",
                    task_domain_id, domain_id
                );
                break;
            }
        }

        i_result.bind(":scan_iteration_id", _iteration_id);
        i_result.bind(":domain_id", task_domain_id);
        i_result.bind(":domain_name", result.domain_name, sqlite3pp::nocopy);
        i_result.bind(":has_keyset", task_has_keyset);
        i_result.bind(":cdnskey_status", result.cdnskey_status, sqlite3pp::nocopy);
        if (result.nameserver)
        {
            i_result.bind(":nameserver", *(result.nameserver), sqlite3pp::nocopy);
        }
        else
        {
            i_result.bind(":nameserver", sqlite3pp::null_type());
        }
        if (result.nameserver_ip)
        {
            i_result.bind(":nameserver_ip", *result.nameserver_ip, sqlite3pp::nocopy);
        }
        else
        {
            i_result.bind(":nameserver_ip", sqlite3pp::nocopy);
        }
        if (result.cdnskey_flags)
        {
            i_result.bind(":cdnskey_flags", *result.cdnskey_flags);
        }
        else
        {
            i_result.bind(":cdnskey_flags", sqlite3pp::null_type());
        }
        if (result.cdnskey_proto)
        {
            i_result.bind(":cdnskey_proto", *result.cdnskey_proto);
        }
        else
        {
            i_result.bind(":cdnskey_proto", sqlite3pp::null_type());
        }
        if (result.cdnskey_alg)
        {
            i_result.bind(":cdnskey_alg", *result.cdnskey_alg);
        }
        else
        {
            i_result.bind(":cdnskey_alg", sqlite3pp::null_type());
        }
        if (result.cdnskey_public_key)
        {
            i_result.bind(":cdnskey_public_key", *result.cdnskey_public_key, sqlite3pp::nocopy);
        }
        else
        {
            i_result.bind(":cdnskey_public_key", sqlite3pp::null_type());
        }
        i_result.step();
        i_result.reset();
    }
    xct.commit();
}


long long SqliteStorage::start_new_scan_iteration() const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);

    sqlite3pp::command i_result(db, "INSERT INTO scan_iteration (end_at) VALUES (NULL)");
    i_result.execute();
    long long iteration_id = db.last_insert_rowid();
    if (iteration_id <= 0)
    {
        throw std::runtime_error("unable to start new iteration");
    }
    xct.commit();
    return iteration_id;
}


void SqliteStorage::end_scan_iteration(const long long _iteration_id) const
{
    if (_iteration_id <= 0)
    {
        throw std::runtime_error("invalid scan iteration id");
    }
    auto db = get_db();
    sqlite3pp::transaction xct(db);

    sqlite3pp::command u_result(db, "UPDATE scan_iteration SET end_at = datetime('now')"
        " WHERE end_at IS NULL AND id = :id"
    );
    u_result.bind(":id", _iteration_id);
    u_result.execute();
    if (db.changes() != 1)
    {
            throw std::runtime_error("unable to mark iteration as finished");
    }
    xct.commit();
}


long long SqliteStorage::prune_finished_scan_queue() const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    db.execute(
        "DELETE FROM scan_queue WHERE id IN"
        " (SELECT sq.id FROM scan_iteration si JOIN scan_result sr ON sr.scan_iteration_id = si.id AND si.end_at IS NOT NULL"
        " JOIN scan_queue sq ON sq.domain_id = sr.domain_id AND sr.scan_at > sq.import_at)"
    );
    xct.commit();
    return db.changes();
}

void SqliteStorage::wipe_unfinished_scan_iterations() const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);

    sqlite3pp::command d_result(db,
        "DELETE FROM scan_result WHERE scan_iteration_id IN"
        " (SELECT id FROM scan_iteration WHERE end_at IS NULL)"
    );
    d_result.execute();
    sqlite3pp::command d2_result(db,
        "DELETE FROM scan_iteration WHERE end_at IS NULL"
    );
    d2_result.execute();

    xct.commit();
}


} // namespace Sqlite
} // namespace Akm
} // namespace Fred
