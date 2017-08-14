#ifndef ENUM_CONVERSION_HH_04D16325B82D363D0CF35195417A0A8C//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define ENUM_CONVERSION_HH_04D16325B82D363D0CF35195417A0A8C

#include <string>

namespace Fred {
namespace Akm {
namespace Sqlite {


template<class T>
T from_db_handle(const std::string& _handle);


} // namespace Sqlite
} // namespace Akm
} // namespace Fred

#endif//ENUM_CONVERSION_HH_04D16325B82D363D0CF35195417A0A8C
