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
#ifndef TYPE_INSTANCE_MAP_D930D58864C914AF17A51B180C0AB639//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define TYPE_INSTANCE_MAP_D930D58864C914AF17A51B180C0AB639

#include <memory>
#include <map>
#include <typeinfo>
#include <typeindex>

namespace Fred {
namespace Akm {

struct TypeNotFound : public std::exception
{
    const char* what() const noexcept { return "type not found"; }
};


template<class TBase, class TNotFoundException = TypeNotFound>
class TypeInstanceMap
{
public:
    void set(std::shared_ptr<TBase> _instance)
    {
        const auto type = std::type_index(typeid(*_instance));
        instances_[type] = _instance;
    }

    template<class T> std::shared_ptr<T> get() const
    {
        const auto type = std::type_index(typeid(T));
        const auto it = instances_.find(type);
        if (it != instances_.end())
        {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        throw TNotFoundException();
    }

private:
    std::map<std::type_index, std::shared_ptr<TBase>> instances_;
};


} // namespace Akm
} // namespace Fred

#endif//TYPE_INSTANCE_MAP_D930D58864C914AF17A51B180C0AB639
