/*
 * Copyright (C) 2017-2018  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef AKM_HH_C1EB76915011A3B8882D64EE0C5D9DB8//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define AKM_HH_C1EB76915011A3B8882D64EE0C5D9DB8

#include "src/corba/nameservice.hh"
#include "src/i_akm.hh"
#include "src/keyset.hh"
#include "src/nsset.hh"
#include "src/email_addresses.hh"

#include <string>
#include <vector>

namespace Fred {
namespace Akm {
namespace Corba {


class Akm : public IAkm
{
public:
    Akm(const Nameservice& _ns, const std::string& _ns_path_akm);

    DomainScanTaskCollection get_nameservers_with_insecure_automatically_managed_domain_candidates() const;

    DomainScanTaskCollection get_nameservers_with_secure_automatically_managed_domain_candidates() const;

    DomainScanTaskCollection get_nameservers_with_automatically_managed_domains() const;

    std::vector<std::string> get_email_addresses_by_domain_id(unsigned long long _domain_id) const;

    void turn_on_automatic_keyset_management_on_insecure_domain(
            unsigned long long _domain_id,
            Nsset _current_nsset,
            Keyset _new_keyset) const;

    void turn_on_automatic_keyset_management_on_secure_domain(
            unsigned long long _domain_id,
            Keyset _new_keyset) const;

    void update_automatically_managed_keyset_of_domain(
            unsigned long long _domain_id,
            Keyset _new_keyset) const;

private:
    const Nameservice& ns_;
    std::string ns_path_akm_;
};


} // namespace Corba
} // namespace Akm
} // namespace Fred

#endif//AKM_HH_C1EB76915011A3B8882D64EE0C5D9DB8
