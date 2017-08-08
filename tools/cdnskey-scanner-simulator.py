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
SECURE_DB = {}

def search_record_insecure(line):
    try:
        nameserver, domains = line.strip().split(' ', 1)
        for domain in domains.split():
            try:
                for record in INSECURE_DB[nameserver][domain]:
                    sys.stdout.write(record + '\n')
            except KeyError as e:
                # or unresolved record to stdout?
                sys.stderr.write('{} not found\n'.format(e))
    except:
        sys.stderr.write('malformed input line ({})\n'.format(line))


def search_record_secure(line):
    try:
        for domain in line.strip().split(' '):
            try:
                for record in SECURE_DB[domain]:
                    sys.stdout.write(record + '\n')
            except KeyError as e:
                sys.stderr.write('{} not found\n'.format(e))
    except:
        sys.stderr.write('malformed input line ({})\n'.format(line))


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
            elif result_type in ('secure', 'secure-empty', 'unknown', 'untrustworthy'):
                result_tokens = tokens.split()
                domain = result_tokens[0]
                SECURE_DB.setdefault(domain, []).append(line)

    # what was loaded from configuration?
    for nameserver, domains in INSECURE_DB.iteritems():
        sys.stderr.write(nameserver + '\n')
        for domain, records in domains.iteritems():
            sys.stderr.write('  ' + domain + '\n')
            for record in records:
                sys.stderr.write(2 * '  ' + record + '\n')

    for domain, records in SECURE_DB.iteritems():
        sys.stderr.write(domain + '\n')
        for record in records:
            sys.stderr.write(2 * '  ' + record + '\n')

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
