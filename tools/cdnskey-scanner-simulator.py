#!/usr/bin/env python

import sys
from enum import Enum

# input
# -----
# [insecure]
# nameserver_1 domain_11 domain_12 ... domain_1n
# nameserver_2 domain_21 domain_22 ... domain_2k
# ...
# [secure]
# domain_1 domain_2 ... domain_i
#
# output
# ------
# insecure nameserver nameserver_ip domain flags proto alg public_key
# insecure-empty nameserver nameserver_ip domain
# unresolved nameserver nameserver_ip domain
# secure domain flags proto alg public_key
# untrustworthy domain
# unknown domain
# secure-empty domain
# unresolved-ip nameserver

# usage in fred-akm.conf
# [scanner]
# tool_path = /usr/bin/python tools/cdnskey-scanner-simulator.py tools/simulator.db.example

ScanType = Enum('ScanType', 'insecure secure')

INSECURE_DB = {}
INSECURE_UNRESOLVED_NS = {}
SECURE_DB = {}


def scanner_output(line):
    sys.stdout.write(line + '\n')


def debug_output(line):
    sys.stderr.write('DEBUG> ' + line + '\n')


def error_output(line):
    sys.stderr.write('ERR> ' + line + '\n')


def search_record_insecure(line):
    try:
        nameserver, domains = line.strip().split(' ', 1)
        if nameserver in INSECURE_UNRESOLVED_NS:
            return
        for domain in domains.split():
            try:
                for record in INSECURE_DB[nameserver][domain]:
                    scanner_output(record)
            except KeyError as e:
                # or unresolved record to stdout?
                error_output('{} not found'.format(e))
    except:
        error_output('malformed input line ({})'.format(line))


def search_record_secure(line):
    try:
        for domain in line.strip().split(' '):
            try:
                for record in SECURE_DB[domain]:
                    scanner_output(record)
            except KeyError as e:
                error_output('{} not found'.format(e))
    except:
        error_output('malformed input line ({})'.format(line))


def search_not_specified(line):
    sys.stderr.write('scan type not specified in input')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(1)

    with open(sys.argv[1]) as input_file:
        for line in input_file.readlines():
            line = line.strip()
            if not line or (len(line) > 0 and line[0] == '#'):
                continue
            result_type, tokens = line.split(' ', 1)
            if result_type in ('insecure', 'insecure-empty', 'unresolved'):
                result_tokens = tokens.split()
                nameserver = result_tokens[0]
                domain = result_tokens[2]
                INSECURE_DB.setdefault(nameserver, {}).setdefault(domain, []).append(line)
            if result_type == 'unresolved-ip':
                result_tokens = tokens.split()
                nameserver = result_tokens[0]
                INSECURE_UNRESOLVED_NS[nameserver] = line
            elif result_type in ('secure', 'secure-empty', 'unknown', 'untrustworthy'):
                result_tokens = tokens.split()
                domain = result_tokens[0]
                SECURE_DB.setdefault(domain, []).append(line)

    # what was loaded from configuration?
    for nameserver, domains in INSECURE_DB.iteritems():
        debug_output(nameserver)
        for domain, records in domains.iteritems():
            debug_output('  ' + domain)
            for record in records:
                debug_output(2 * '  ' + record)

    for nameserver, record in INSECURE_UNRESOLVED_NS.iteritems():
        debug_output(nameserver)
        debug_output('  ' + record)

    for domain, records in SECURE_DB.iteritems():
        debug_output(domain)
        for record in records:
            debug_output(2 * '  ' + record)

    # put unresolved-ip records to output before reading input
    # need just one unresolved-ip result per nameserver to simulate cdnskey-scanner behaviour
    for nameserver, record in INSECURE_UNRESOLVED_NS.iteritems():
        scanner_output(record)

    scan_type = None
    for line in sys.stdin.readlines():
        line = line.strip()
        if line == '[insecure]':
            scan_type = ScanType.insecure
        elif line == '[secure]':
            scan_type = ScanType.secure
        else:
            call = {
                ScanType.insecure: search_record_insecure,
                ScanType.secure: search_record_secure,
                None: search_not_specified
            }.get(scan_type)(line)
