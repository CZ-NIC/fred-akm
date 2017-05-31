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
