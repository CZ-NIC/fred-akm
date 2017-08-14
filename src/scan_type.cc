#include "src/scan_type.hh"
#include "src/sqlite/scan_type_conversion.hh"

namespace Fred {
namespace Akm {


std::string to_string(ScanType _scan_type)
{
    /* for now use same conversion as to db handle */
    return Sqlite::to_db_handle(_scan_type);
}


template<>
ScanType from_string(const std::string& _str)
{
    /* for now use same conversion as from db handle */
    return Sqlite::from_db_handle<ScanType>(_str);
}


} // namespace Akm
} // namespace Fred


