#include "src/sqlite/storage.hh"
#include "src/sqlite/scan_type_conversion.hh"
#include "src/log.hh"

#include "src/domain_state.hh"
#include "src/domain_status.hh"
#include "src/enum_conversions.hh"
#include "src/notification_type.hh"
#include "src/notified_domain_status.hh"
#include "src/scan_result_row.hh"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>

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
        "CREATE TABLE IF NOT EXISTS enum_scan_type ("
        " id INTEGER PRIMARY KEY NOT NULL,"
        " handle TEXT NOT NULL)"
    );
    _db.execute("CREATE UNIQUE INDEX enum_scan_type_uniq_idx ON enum_scan_type(handle)");
    _db.execute("INSERT OR REPLACE INTO enum_scan_type (id, handle) VALUES (1, 'insecure')");
    _db.execute("INSERT OR REPLACE INTO enum_scan_type (id, handle) VALUES (2, 'secure-auto')");

    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_queue ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " import_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " nameserver TEXT NOT NULL,"
            " domain_id INTEGER NOT NULL CHECK(domain_id > 0),"
            " domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''),"
            " scan_type_id INTEGER NOT NULL REFERENCES enum_scan_type(id))"
    );
    _db.execute("CREATE INDEX scan_queue_ns_domain_idx ON scan_queue(nameserver, domain_id, domain_name)");
    _db.execute("CREATE INDEX scan_queue_domain_name_idx ON scan_queue(domain_name)");
    _db.execute("CREATE INDEX scan_queue_scan_type_id_idx ON scan_queue(scan_type_id)");
}


void create_schema_scan_result(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS scan_result ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
            " scan_iteration_id INTEGER NOT NULL,"
            " scan_at TEXT NOT NULL DEFAULT (datetime('now')),"
            " domain_id INTEGER CHECK(domain_id > 0),"
            " domain_name TEXT,"
            " scan_type_id INTEGER NOT NULL REFERENCES enum_scan_type(id),"
            " cdnskey_status TEXT NOT NULL CHECK(cdnskey_status IN ('insecure','insecure-empty','unresolved','secure','secure-empty','untrustworthy','unknown','unresolved-ip')),"
            " nameserver TEXT,"
            " nameserver_ip TEXT,"
            " cdnskey_flags INTEGER,"
            " cdnskey_proto INTEGER,"
            " cdnskey_alg INTEGER,"
            " cdnskey_public_key TEXT,"
            " FOREIGN KEY (scan_iteration_id) REFERENCES scan_iteration(id))"
    );
    _db.execute("CREATE INDEX scan_result_scan_type_id_idx ON scan_result(scan_type_id)");
}


void create_schema_domain_status_notification(sqlite3pp::database& _db)
{
    _db.execute(
        "CREATE TABLE IF NOT EXISTS domain_status_notification ( "
            "domain_id INTEGER PRIMARY KEY NOT NULL CHECK(domain_id > 0), "
            "domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''), "
            "scan_type_id INTEGER NOT NULL REFERENCES enum_scan_type(id),"
            "cdnskeys TEXT, "
            "domain_status INTEGER NOT NULL, "
            "notification_type INTEGER NOT NULL, "
            "last_at TEXT NOT NULL DEFAULT (datetime('now')))"
    );
    _db.execute("CREATE INDEX domain_status_notification_scan_type_id_idx ON domain_status_notification(scan_type_id)");
}


void create_schema(sqlite3pp::database& _db)
{

    create_schema_scan_iteration(_db);
    create_schema_scan_queue(_db);
    create_schema_scan_result(_db);
    create_schema_domain_status_notification(_db);
}


void drop_schema(sqlite3pp::database& _db)
{
    _db.execute("DROP TABLE IF EXISTS scan_queue");
    _db.execute("DROP TABLE IF EXISTS scan_result");
    _db.execute("DROP TABLE IF EXISTS scan");
    _db.execute("DROP TABLE IF EXISTS scan_iteration");
    _db.execute("DROP TABLE IF EXISTS domain_status_notification");
}


}


namespace Impl {


void append_to_scan_queue(sqlite3pp::database& _db, const DomainScanTaskCollection& _data)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO scan_queue (nameserver, domain_id, domain_name, scan_type_id)"
        " VALUES (?, ?, ?, (SELECT id FROM enum_scan_type WHERE handle = ?))"
    );

    for (const auto& domain_scan_task : _data)
    {
        const Domain& domain = domain_scan_task.domain;
        for (const auto& nameserver : domain_scan_task.nameservers)
        {
            insert.binder() << nameserver << static_cast<long long>(domain.id) << domain.fqdn << to_db_handle(domain.scan_type);
            insert.step();
            insert.reset();
        }
    }
}


void append_to_scan_queue_if_not_exists(sqlite3pp::database& _db, const DomainScanTaskCollection& _data)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT INTO scan_queue (nameserver, domain_id, domain_name, scan_type_id)"
        " VALUES (:nameserver, :domain_id, :domain_name, (SELECT id FROM enum_scan_type WHERE handle = :scan_type))"
    );
    sqlite3pp::query check(_db);
    check.prepare(
        "SELECT 1 FROM scan_queue WHERE"
        " nameserver = :nameserver AND domain_id = :domain_id AND domain_name = :domain_name"
    );
    for (const auto& domain_scan_task : _data)
    {
        const Domain& domain = domain_scan_task.domain;
        for (const auto& nameserver : domain_scan_task.nameservers)
        {
            check.bind(":nameserver", nameserver, sqlite3pp::nocopy);
            check.bind(":domain_id", static_cast<long long>(domain.id));
            check.bind(":domain_name", domain.fqdn, sqlite3pp::nocopy);
            if (check.begin() == check.end())
            {
                insert.bind(":nameserver", nameserver, sqlite3pp::nocopy);
                insert.bind(":domain_id", static_cast<long long>(domain.id));
                insert.bind(":domain_name", domain.fqdn, sqlite3pp::nocopy);
                insert.bind(":scan_type", to_db_handle(domain.scan_type), sqlite3pp::nocopy);
                insert.step();
                insert.reset();
            }
            check.reset();
        }
    }
}


ScanResultRows get_insecure_scan_result_rows_for_notify(
        sqlite3pp::database& _db, const int _seconds_back,
        const bool _notify_from_last_iteration_only,
        const bool _align_to_start_of_day)
{
    sqlite3pp::query query(_db);
    query.prepare(boost::str(boost::format(
        "SELECT scan_result.id, "
               "scan_result.scan_iteration_id, "
               "COALESCE(scan_result.scan_at, ''), "
               "strftime('%%s', datetime(scan_at)) AS scan_at_seconds, "
               "scan_result.domain_id, "
               "COALESCE(scan_result.domain_name, ''), "
               "enum_scan_type.handle AS scan_type, "
               "COALESCE(scan_result.cdnskey_status, ''), "
               "COALESCE(scan_result.nameserver, ''), "
               "COALESCE(scan_result.nameserver_ip, ''), "
               "scan_result.cdnskey_flags, "
               "scan_result.cdnskey_proto, "
               "scan_result.cdnskey_alg, "
               "COALESCE(scan_result.cdnskey_public_key, '') "
          "FROM scan_result "
          "JOIN enum_scan_type ON enum_scan_type.id = scan_result.scan_type_id "
          "LEFT JOIN domain_status_notification ON scan_result.domain_id = domain_status_notification.domain_id "
         "WHERE scan_result.scan_iteration_id IN "
             "(SELECT scan_iteration_id "
                "FROM scan_result "
               "WHERE scan_at >= datetime('now', '%1% seconds', '" + std::string(_align_to_start_of_day ? "start of day" : "0 seconds") + "') "
               "GROUP BY scan_iteration_id) " // always get all scan_results from concrete iteration_id
           "AND enum_scan_type.handle = 'insecure' "
           "AND (scan_result.scan_at > domain_status_notification.last_at OR domain_status_notification.last_at IS NULL) "
           "%2%"
         "ORDER BY scan_result.id DESC")
                   % (_seconds_back * -1)
                   % (_notify_from_last_iteration_only ? "AND scan_iteration_id = (SELECT MAX(scan_iteration_id) FROM scan_result) " : "")).c_str());

    // Note: if (query.begin() == query.end()) increments internal pointer, do not use here!

    ScanResultRows scan_result;

    boost::optional<ScanResultRow> last_scan_result_row;
    for (auto row : query)
    {
        std::string scan_type_handle;
        ScanResultRow scan_result_row;
        try {
            row.getter()
                    >> scan_result_row.id
                    >> scan_result_row.scan_iteration_id
                    >> scan_result_row.scan_at
                    >> scan_result_row.scan_at_seconds
                    >> scan_result_row.domain_id
                    >> scan_result_row.domain_name
                    >> scan_type_handle
                    >> scan_result_row.cdnskey.status
                    >> scan_result_row.nameserver
                    >> scan_result_row.nameserver_ip
                    >> scan_result_row.cdnskey.flags
                    >> scan_result_row.cdnskey.proto
                    >> scan_result_row.cdnskey.alg
                    >> scan_result_row.cdnskey.public_key;

            scan_result_row.scan_type = from_db_handle<ScanType>(scan_type_handle);
            scan_result.emplace_back(scan_result_row);
            //log()->debug(to_string(scan_result_row));
        }
        catch (...)
        {
            log()->error("FAILED to get a scan_result_row from the database: {}", to_string(scan_result_row));
        }
    }
    //if (scan_result.empty()) {
    //    throw std::runtime_error("no scan_results found");
    //}
    return scan_result;
}

ScanResultRows get_insecure_scan_result_rows_for_update(
        sqlite3pp::database& _db,
        const int _seconds_back,
        const bool _align_to_start_of_day)
{
    const ScanType scan_type = ScanType::insecure;
    const NotificationType notification_type = NotificationType::akm_notification_candidate_ok;
    const NotificationType notification_type_fallen_angel = NotificationType::akm_notification_managed_ok;

    sqlite3pp::query query(_db);
    std::string sql =
        "SELECT scan_result.id, "
               "scan_result.scan_iteration_id, "
               "COALESCE(scan_result.scan_at, ''), "
               "strftime('%%s', datetime(scan_at)) AS scan_at_seconds, "
               "scan_result.domain_id, "
               "COALESCE(scan_result.domain_name, ''), "
               "enum_scan_type.handle AS scan_type, "
               "COALESCE(scan_result.cdnskey_status, ''), "
               "COALESCE(scan_result.nameserver, ''), "
               "COALESCE(scan_result.nameserver_ip, ''), "
               "scan_result.cdnskey_flags, "
               "scan_result.cdnskey_proto, "
               "scan_result.cdnskey_alg, "
               "COALESCE(scan_result.cdnskey_public_key, '') "
          "FROM scan_result "
          "JOIN enum_scan_type ON enum_scan_type.id = scan_result.scan_type_id "
          "JOIN domain_status_notification ON domain_status_notification.domain_id = scan_result.domain_id "
         "WHERE scan_iteration_id IN "
             "(SELECT scan_iteration_id "
                "FROM scan_result "
               "WHERE scan_at >= datetime('now', '%1% seconds', '" + std::string(_align_to_start_of_day ? "start of day" : "0 seconds") + "') "
               "GROUP BY scan_iteration_id) " // always get all scan_results from concrete iteration_id
           "AND enum_scan_type.handle = :scan_type "
           "AND (domain_status_notification.notification_type = :notification_type "
           "OR domain_status_notification.notification_type = :notification_type_fallen_angel) "
           "AND domain_status_notification.last_at < datetime('now', '%2% seconds', '" + std::string(_align_to_start_of_day ? "start of day" : "0 seconds") + "') "
         "ORDER BY scan_result.id DESC";

    query.prepare(boost::str(boost::format(sql)
                   % (_seconds_back * -1)
                   % (_seconds_back * -1)).c_str());

    query.bind(":scan_type", to_db_handle(scan_type), sqlite3pp::nocopy);
    query.bind(":notification_type", to_db_handle(notification_type));
    query.bind(":notification_type_fallen_angel", to_db_handle(notification_type_fallen_angel));

    // Note: if (query.begin() == query.end()) increments internal pointer, do not use here!

    ScanResultRows scan_result;

    boost::optional<ScanResultRow> last_scan_result_row;
    for (auto row : query)
    {
        std::string scan_type_handle;
        ScanResultRow scan_result_row;
        try {
            row.getter()
                    >> scan_result_row.id
                    >> scan_result_row.scan_iteration_id
                    >> scan_result_row.scan_at
                    >> scan_result_row.scan_at_seconds
                    >> scan_result_row.domain_id
                    >> scan_result_row.domain_name
                    >> scan_type_handle
                    >> scan_result_row.cdnskey.status
                    >> scan_result_row.nameserver
                    >> scan_result_row.nameserver_ip
                    >> scan_result_row.cdnskey.flags
                    >> scan_result_row.cdnskey.proto
                    >> scan_result_row.cdnskey.alg
                    >> scan_result_row.cdnskey.public_key;

            scan_result_row.scan_type = from_db_handle<ScanType>(scan_type_handle);
            scan_result.emplace_back(scan_result_row);
            //log()->debug(to_string(scan_result_row));
        }
        catch (...)
        {
            log()->error("FAILED to get a scan_result_row from the database: {}", to_string(scan_result_row));
        }
    }
    //if (scan_result.empty()) {
    //    throw std::runtime_error("no scan_results found");
    //}
    return scan_result;
}

ScanResultRows get_secure_scan_result_rows_for_update(
        sqlite3pp::database& _db,
        const int _seconds_back,
        const bool _align_to_start_of_day)
{
    const ScanType scan_type = ScanType::secure_auto;
    const NotificationType notification_type = NotificationType::akm_notification_managed_ok;

    sqlite3pp::query query(_db);
    std::string sql =
        "SELECT scan_result.id, "
               "scan_result.scan_iteration_id, "
               "COALESCE(scan_result.scan_at, ''), "
               "strftime('%%s', datetime(scan_at)) AS scan_at_seconds, "
               "scan_result.domain_id, "
               "COALESCE(scan_result.domain_name, ''), "
               "enum_scan_type.handle AS scan_type, "
               "COALESCE(scan_result.cdnskey_status, ''), "
               "COALESCE(scan_result.nameserver, ''), "
               "scan_result.cdnskey_flags, "
               "scan_result.cdnskey_proto, "
               "scan_result.cdnskey_alg, "
               "COALESCE(scan_result.cdnskey_public_key, '') "
          "FROM scan_result "
          "JOIN enum_scan_type ON enum_scan_type.id = scan_result.scan_type_id "
          "LEFT JOIN domain_status_notification ON domain_status_notification.domain_id = scan_result.domain_id "
         "WHERE scan_iteration_id IN "
             "(SELECT scan_iteration_id "
                "FROM scan_result "
               "WHERE scan_at >= datetime('now', '%1% seconds', '" + std::string(_align_to_start_of_day ? "start of day" : "0 seconds") + "') "
               "GROUP BY scan_iteration_id) " // always get all scan_results from concrete iteration_id
           "AND scan_result.scan_type = :scan_type "
           "AND (domain_status_notification.notification_type = :notification_type OR domain_status_notification.notification_type IS NULL) "
           "AND (scan_result.scan_at > domain_status_notification.last_at OR domain_status_notification.last_at IS NULL) "
         "ORDER BY scan_result.id DESC";

    query.prepare(boost::str(boost::format(sql)
                   % (_seconds_back * -1)).c_str());

    query.bind(":scan_type", to_db_handle(scan_type), sqlite3pp::nocopy);
    query.bind(":notification_type", to_db_handle(notification_type));

    // Note: if (query.begin() == query.end()) increments internal pointer, do not use here!

    ScanResultRows scan_result;

    boost::optional<ScanResultRow> last_scan_result_row;
    for (auto row : query)
    {
        std::string scan_type_handle;
        ScanResultRow scan_result_row;
        try {
            row.getter()
                    >> scan_result_row.id
                    >> scan_result_row.scan_iteration_id
                    >> scan_result_row.scan_at
                    >> scan_result_row.scan_at_seconds
                    >> scan_result_row.domain_id
                    >> scan_result_row.domain_name
                    >> scan_type_handle
                    >> scan_result_row.cdnskey.status
                    >> scan_result_row.nameserver
                    >> scan_result_row.cdnskey.flags
                    >> scan_result_row.cdnskey.proto
                    >> scan_result_row.cdnskey.alg
                    >> scan_result_row.cdnskey.public_key;

            scan_result_row.scan_type = from_db_handle<ScanType>(scan_type_handle);
            scan_result.emplace_back(scan_result_row);
        }
        catch (...)
        {
            log()->error("FAILED to get a scan_result_row from the database: {}", to_string(scan_result_row));
        }
    }
    //if (scan_result.empty()) {
    //    throw std::runtime_error("no scan_results found");
    //}
    return scan_result;
}

void set_notified_domain_status(sqlite3pp::database& _db, const NotifiedDomainStatus& _notified_domain_status)
{
    sqlite3pp::command insert(_db);
    insert.prepare(
        "INSERT OR REPLACE INTO domain_status_notification"
            " (domain_id, domain_name, scan_type_id, cdnskeys, domain_status, notification_type, last_at)"
            " VALUES (?, ?, (SELECT id FROM enum_scan_type WHERE handle = ?), ?, ?, ?, ?)");
    insert.binder()
            << static_cast<long long>(_notified_domain_status.domain.id)
            << _notified_domain_status.domain.fqdn
            << to_db_handle(_notified_domain_status.domain.scan_type)
            << _notified_domain_status.serialized_cdnskeys
            << to_db_handle(_notified_domain_status.domain_status)
            << to_db_handle(_notified_domain_status.notification_type)
            << _notified_domain_status.last_at;
    insert.execute();
}

boost::optional<NotifiedDomainStatus> get_last_notified_domain_status(sqlite3pp::database& _db, const unsigned long long _domain_id)
{
    sqlite3pp::query query(_db);
    query.prepare(
        "SELECT domain_id, "
               "domain_name, "
               "enum_scan_type.handle AS scan_type, "
               "cdnskeys, "
               "domain_status, "
               "notification_type, "
               "last_at, "
               "strftime('%s', datetime(last_at)) AS last_at_seconds "
          "FROM domain_status_notification "
          "JOIN enum_scan_type ON enum_scan_type.id = domain_status_notification.scan_type_id "
         "WHERE domain_id = ?");

    query.bind(1, static_cast<long long>(_domain_id));

    // how to check that the query is empty?
    // query.begin() == query.end() has some side-effect, if used, following (*query.begin()).getter() does not work
    // solution for now:
    for (auto row : query)
    {
        NotifiedDomainStatus notified_domain_status;
        long long domain_id;
        int domain_status;
        int notification_type;
        std::string scan_type_handle;
        row.getter()
            >> domain_id
            >> notified_domain_status.domain.fqdn
            >> scan_type_handle
            >> notified_domain_status.serialized_cdnskeys
            >> domain_status
            >> notification_type
            >> notified_domain_status.last_at
            >> notified_domain_status.last_at_seconds;
        notified_domain_status.domain.id = static_cast<unsigned long long>(domain_id);
        notified_domain_status.domain.scan_type = from_db_handle<ScanType>(scan_type_handle);
        notified_domain_status.domain_status = Conversion::Enums::from_db_handle<DomainStatus::DomainStatusType>(domain_status);
        notified_domain_status.notification_type = Conversion::Enums::from_db_handle<NotificationType>(notification_type);
        return notified_domain_status;
    }

    // if we got here, query was empty
    return boost::optional<NotifiedDomainStatus>();
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


void SqliteStorage::append_to_scan_queue(const DomainScanTaskCollection& _data) const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::append_to_scan_queue(db, _data);
    xct.commit();
}


void SqliteStorage::append_to_scan_queue_if_not_exists(const DomainScanTaskCollection& _data) const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::append_to_scan_queue_if_not_exists(db, _data);
    xct.commit();
}


ScanResultRows SqliteStorage::get_insecure_scan_result_rows_for_notify(
        const int _seconds_back,
        const bool _notify_from_last_iteration_only,
        const bool _align_to_start_of_day) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    return Impl::get_insecure_scan_result_rows_for_notify(
            db,
            _seconds_back,
            _notify_from_last_iteration_only,
            _align_to_start_of_day);
}

ScanResultRows SqliteStorage::get_insecure_scan_result_rows_for_update(
        const int _seconds_back,
        const bool _align_to_start_of_day) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    return Impl::get_insecure_scan_result_rows_for_update(
            db,
            _seconds_back,
            _align_to_start_of_day);
}

ScanResultRows SqliteStorage::get_secure_scan_result_rows_for_update(
        const int _seconds_back,
        const bool _align_to_start_of_day) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    return Impl::get_secure_scan_result_rows_for_update(
            db,
            _seconds_back,
            _align_to_start_of_day);
}

void SqliteStorage::set_notified_domain_status(const NotifiedDomainStatus& _notified_domain_status) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    Impl::set_notified_domain_status(db, _notified_domain_status);
    xct.commit();
}

boost::optional<NotifiedDomainStatus> SqliteStorage::get_last_notified_domain_status(
        const unsigned long long _domain_id) const
{
    sqlite3pp::database db(filename_.c_str());
    sqlite3pp::transaction xct(db);
    create_schema(db);
    return Impl::get_last_notified_domain_status(
            db,
            _domain_id);
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


DomainScanTaskCollection SqliteStorage::get_scan_queue_tasks() const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    sqlite3pp::query tasks_query(
        db,
        "SELECT nameserver, domain_id, domain_name, enum_scan_type.handle AS scan_type"
        " FROM scan_queue"
        " JOIN enum_scan_type ON enum_scan_type.id = scan_queue.scan_type_id"
        " ORDER BY nameserver, domain_name ASC"
    );

    DomainScanTaskCollection scan_tasks;
    for (auto row : tasks_query)
    {
        std::string nameserver;
        Domain domain;
        long long domain_id;
        std::string scan_type_handle;
        row.getter() >> nameserver >> domain_id >> domain.fqdn >> scan_type_handle;
        domain.id = static_cast<unsigned long long>(domain_id);
        domain.scan_type = from_db_handle<ScanType>(scan_type_handle);

        scan_tasks.insert_or_update(domain, nameserver);
    }

    return scan_tasks;
}


void SqliteStorage::save_scan_results(const ScanResults& _results, const DomainScanTaskCollection& _tasks, long long _iteration_id) const
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
        " (scan_iteration_id, domain_id, domain_name, scan_type_id, cdnskey_status,"
        " nameserver, nameserver_ip, cdnskey_flags, cdnskey_proto, cdnskey_alg, cdnskey_public_key)"
        " VALUES"
            " (:scan_iteration_id, :domain_id, :domain_name,"
            " (SELECT id FROM enum_scan_type WHERE handle = :scan_type),"
            " :cdnskey_status, :nameserver, :nameserver_ip,"
            " :cdnskey_flags, :cdnskey_proto, :cdnskey_alg, :cdnskey_public_key)"
    );

    const NameserverToDomainScanTaskAdapter tasks_by_nameserver(_tasks);

    for (const auto& result : _results)
    {
        std::vector<const Domain*> result_for_domains;

        if (result.nameserver)
        {
            if (result.domain_name.length())
            {
                const auto& domain = _tasks.find(result.domain_name).domain;
                result_for_domains.push_back(&domain);
            }
            else
            {
                for (const auto task : tasks_by_nameserver.find_all(*result.nameserver))
                {
                    const auto& domain = task->domain;
                    result_for_domains.push_back(&domain);
                }
            }
        }
        else
        {
            if (result.domain_name.length())
            {
                const auto& domain = _tasks.find(result.domain_name).domain;
                // XXX: for all nameservers?
                result_for_domains.push_back(&domain);
            }
        }

        for (const auto& domain : result_for_domains)
        {

            i_result.bind(":scan_iteration_id", _iteration_id);
            i_result.bind(":scan_type", to_db_handle(domain->scan_type), sqlite3pp::nocopy);
            i_result.bind(":cdnskey_status", result.cdnskey_status, sqlite3pp::nocopy);
            if (domain->id != 0)
            {
                i_result.bind(":domain_id", static_cast<long long>(domain->id));
            }
            else
            {
                i_result.bind(":domain_id", sqlite3pp::null_type());
            }
            if (result.domain_name.length())
            {
                i_result.bind(":domain_name", result.domain_name, sqlite3pp::nocopy);
            }
            else
            {
                if (domain->fqdn.length())
                {
                    i_result.bind(":domain_name", domain->fqdn, sqlite3pp::nocopy);
                }
                else
                {
                    i_result.bind(":domain_name", sqlite3pp::null_type());
                }
            }
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
                i_result.bind(":nameserver_ip", *(result.nameserver_ip), sqlite3pp::nocopy);
            }
            else
            {
                i_result.bind(":nameserver_ip", sqlite3pp::null_type());
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


void SqliteStorage::clean_scan_results(
        const int _keep_seconds_back,
        const bool _align_to_start_of_day) const
{
    auto db = get_db();
    sqlite3pp::transaction xct(db);
    sqlite3pp::query query(db);
    create_schema_scan_iteration(db);
    create_schema_scan_result(db);
    query.prepare(boost::str(boost::format(
        "DELETE FROM scan_result "
         "WHERE scan_iteration_id IN ("
            "SELECT id FROM scan_iteration "
             "WHERE end_at < datetime('now', '%1% seconds', '" + std::string(_align_to_start_of_day ? "start of day" : "0 seconds") + "')")
                   % (_keep_seconds_back * -1)).c_str());
    xct.commit();
}


int SqliteStorage::get_current_unix_time() const
{
    auto db = get_db();
    sqlite3pp::query query(db);
    query.prepare(
        "SELECT strftime('%s', datetime('now')) AS current_unix_time"
    );
    int current_unix_time;
    for (auto row : query)
    {
        try {
            row.getter() >> current_unix_time;
            return current_unix_time;
        }
        catch (...)
        {
            break;
        }
    }
    log()->error("FAILED to get current unix time from the database");
    throw std::runtime_error("FAILED to get current unix time from the database");
}

} // namespace Sqlite
} // namespace Akm
} // namespace Fred
