#ifndef SCAN_TYPE_HH_ECE5FCCE435D2BCBF7E34A3C3D9C6259//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define SCAN_TYPE_HH_ECE5FCCE435D2BCBF7E34A3C3D9C6259

#include "src/from_string.hh"

#include <string>

namespace Fred {
namespace Akm {


enum class ScanType
{
    insecure,
    secure_auto,
    secure_noauto
};

std::string to_string(ScanType _scan_type);

template<> ScanType from_string(const std::string& _str);


} // namespace Akm
} // namespace Fred

#endif//SCAN_TYPE_HH_ECE5FCCE435D2BCBF7E34A3C3D9C6259
