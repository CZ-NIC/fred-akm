#ifndef UTILS_HH_D4018E84191F90068B09B1FB56C850C7//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define UTILS_HH_D4018E84191F90068B09B1FB56C850C7

#include <vector>
#include <string>

namespace Fred {
namespace Akm {


std::vector<std::string> split_on(const std::string& _string, const char delimiter);


} // namespace Akm
} // namespace Fred

#endif//UTILS_HH_D4018E84191F90068B09B1FB56C850C7
