#ifndef OUTPUT_PARSER_HH_DDEC7B4468DE682C3826F9CD09E6C08B//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define OUTPUT_PARSER_HH_DDEC7B4468DE682C3826F9CD09E6C08B

#include "src/i_scanner.hh"

#include <string>


namespace Fred {
namespace Akm {


class ScanResultParser
{
public:
    ScanResult parse(const std::string& _line) const;

    std::vector<ScanResult> parse_multi(std::string& _multi_line) const;
};


} // namespace Akm
} // namespace Fred

#endif//OUTPUT_PARSER_HH_DDEC7B4468DE682C3826F9CD09E6C08B
