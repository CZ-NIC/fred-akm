BEGIN;

---
--- Clean old records to make migration faster
---
DELETE FROM scan_result WHERE scan_iteration_id IN (
    SELECT id FROM scan_iteration WHERE end_at < datetime('now', '-9 days')
);


---
---
--- Table backup - rename
---
ALTER TABLE scan_queue RENAME TO scan_queue_backup;
ALTER TABLE scan_result RENAME TO scan_result_backup;
ALTER TABLE domain_status_notification RENAME TO domain_status_notification_backup;

DROP INDEX scan_queue_ns_domain_idx;
DROP INDEX scan_queue_domain_name_idx;

---
--- Create new schema
---
CREATE TABLE IF NOT EXISTS enum_scan_type (
    id INTEGER PRIMARY KEY NOT NULL,
    handle TEXT NOT NULL
);

CREATE UNIQUE INDEX enum_scan_type_uniq_idx ON enum_scan_type(handle);
INSERT OR REPLACE INTO enum_scan_type (id, handle) VALUES (1, 'insecure');
INSERT OR REPLACE INTO enum_scan_type (id, handle) VALUES (2, 'secure-auto');
INSERT OR REPLACE INTO enum_scan_type (id, handle) VALUES (3, 'secure-noauto');

CREATE TABLE IF NOT EXISTS scan_queue (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    import_at TEXT NOT NULL DEFAULT (datetime('now')),
    nameserver TEXT NOT NULL,
    domain_id INTEGER NOT NULL CHECK(domain_id > 0),
    domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''),
    scan_type_id INTEGER NOT NULL REFERENCES enum_scan_type(id)
);

CREATE INDEX scan_queue_ns_domain_idx ON scan_queue(nameserver, domain_id, domain_name);
CREATE INDEX scan_queue_domain_name_idx ON scan_queue(domain_name);
CREATE INDEX scan_queue_scan_type_id_idx ON scan_queue(scan_type_id);

CREATE TABLE IF NOT EXISTS scan_result (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    scan_iteration_id INTEGER NOT NULL,
    scan_at TEXT NOT NULL DEFAULT (datetime('now')),
    domain_id INTEGER CHECK(domain_id > 0),
    domain_name TEXT,
    scan_type_id INTEGER NOT NULL REFERENCES enum_scan_type(id),
    cdnskey_status TEXT NOT NULL CHECK(cdnskey_status IN ('insecure','insecure-empty','unresolved','secure','secure-empty','untrustworthy','unknown','unresolved-ip')),
    nameserver TEXT,
    nameserver_ip TEXT,
    cdnskey_flags INTEGER,
    cdnskey_proto INTEGER,
    cdnskey_alg INTEGER,
    cdnskey_public_key TEXT,
    FOREIGN KEY (scan_iteration_id) REFERENCES scan_iteration(id)
);
CREATE INDEX scan_result_scan_type_id_idx ON scan_result(scan_type_id);

CREATE TABLE IF NOT EXISTS domain_status_notification (
    domain_id INTEGER PRIMARY KEY NOT NULL CHECK(domain_id > 0),
    domain_name TEXT NOT NULL CHECK(COALESCE(domain_name, '') != ''),
    scan_type_id INTEGER NOT NULL REFERENCES enum_scan_type(id),
    cdnskeys TEXT,
    domain_status INTEGER NOT NULL,
    notification_type INTEGER NOT NULL,
    last_at TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE INDEX domain_status_notification_scan_type_id_idx ON domain_status_notification(scan_type_id);


---
--- Data migration
--- No need to migrade data scan_queue (will be loaded in next run)
---
INSERT INTO scan_result
    SELECT id,
           scan_iteration_id,
           scan_at,
           domain_id,
           domain_name,
           has_keyset + 1,
           cdnskey_status,
           nameserver,
           nameserver_ip,
           cdnskey_flags,
           cdnskey_proto,
           cdnskey_alg,
           cdnskey_public_key
      FROM scan_result_backup;

INSERT INTO domain_status_notification
    SELECT domain_id,
           domain_name,
           has_keyset + 1,
           cdnskeys,
           domain_status,
           notification_type,
           last_at
      FROM domain_status_notification_backup;


---
--- Basic checks
---
SELECT l.count = r.count
  FROM (SELECT count(*) AS count
          FROM scan_result_backup) AS l,
       (SELECT count(*) AS count
          FROM scan_result) AS r;

SELECT l.count = r.count
  FROM (SELECT count(*) AS count
          FROM domain_status_notification_backup) AS l,
       (SELECT count(*) AS count
          FROM domain_status_notification) AS r;
