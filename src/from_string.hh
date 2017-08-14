#ifndef FROM_STRING_HH_3AC40C9B47AC09214CE546C9737AEA9D//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define FROM_STRING_HH_3AC40C9B47AC09214CE546C9737AEA9D

#include <string>

namespace Fred {
namespace Akm {

template<class T> T from_string(const std::string&);

} // namespace Akm
} // namespace Fred

#endif//FROM_STRING_HH_3AC40C9B47AC09214CE546C9737AEA9D
